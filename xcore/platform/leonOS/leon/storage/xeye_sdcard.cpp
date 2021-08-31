/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <rtems/fsmount.h>
#include <rtems/bdpart.h>
#include "OsDrvSdioDefines.h"
#include "OsDrvSdio.h"
#include "xeye_sdcard.h"
#include "platform_pubdef.h"
#define MVLOG_UNIT_NAME xeye_sdcard
#include <mvLog.h>

#define MODEL_GRAPH             "/mnt/sdcard/graph"
#define MODEL_GRAPHLIST         "/mnt/sdcard/graphlist"
#define SDIO_DEVNAME_USED       "/dev/sdc0"
// TODO(zhoury): wait to refactor graph relative struct
extern u8 lrt_graphBuf[];
extern int lrt_graphBufLen;
extern u32 lrt_modelNum;
extern t_graph lrt_g_graphList[];

static const rtems_fstab_entry fs_table [] = {
    {
        .source = "/dev/sdc0",
        .target = "/mnt/sdcard",
        .type = "dosfs",
        .options = RTEMS_FILESYSTEM_READ_WRITE,
        .report_reasons = RTEMS_FSTAB_NONE,
        .abort_reasons = RTEMS_FSTAB_OK
    },
    {
        .source = "/dev/sdc01",
        .target = "/mnt/sdcard",
        .type = "dosfs",
        .options = RTEMS_FILESYSTEM_READ_WRITE,
        .report_reasons = RTEMS_FSTAB_NONE,
        .abort_reasons = RTEMS_FSTAB_NONE
    }
};

// init sd card
int XeyeDrvSdcardInit(void) {
    int sc;

    osDrvSdioEntries_t info = { 1,  // Number of slots
                                10, // Interrupt priority
    {   {
            1, // Card slot to be used
            "/dev/sdc0", // Device name
            SDIO_SDR50, // Max speed mode
            NULL
        }
    }
                              };

    sc = OsDrvSdioInit(&info);
    mvLog(MVLOG_INFO, "OsDrvSdioInit: %s", rtems_status_text(static_cast<rtems_status_code>(sc)));
    if (sc) {
        mvLog(MVLOG_ERROR, "Sdio Drv Init Failed");
        return sc;
    }

    sc = rtems_bdpart_register_from_disk(SDIO_DEVNAME_USED);
    mvLog(MVLOG_INFO, "rtems_bdpart_register_from_disk: %s", rtems_status_text(static_cast<rtems_status_code>(sc)));
    if (sc) {
        mvLog(MVLOG_ERROR, "Detect NO Sdcard");
        return sc;
    }

    sc = rtems_fsmount(fs_table, sizeof(fs_table) / sizeof(fs_table[0]), NULL);
    mvLog(MVLOG_INFO, "Mounting File System: %s", rtems_status_text(static_cast<rtems_status_code>(sc)));
    if (sc) {
        mvLog(MVLOG_ERROR, "Mounting File System Failed");
        return sc;
    }
    return 0;
}

// read file to buffer
int readFileFromSdCard(char* fileName, char* pBuffer, int iLen) {
    int fd = 0;
    int ret = 0;
    struct stat fileStat;

    // Validate written data
    fd = open(fileName, O_RDONLY);

    if (fd < 0) {
        mvLog(MVLOG_ERROR, "open sdcard file %s failed", fileName);
        return -1;
    }

    memset(&fileStat, 0, sizeof(struct stat));
    ret = fstat(fd, &fileStat);

    if (ret < 0 || fileStat.st_size > iLen) {
        mvLog(MVLOG_ERROR, "sdcard read failed as buffer size not enough");
        close(fd);
        return -1;
    }

    ret = read(fd, pBuffer, iLen);

    if (ret <= 0) {
        mvLog(MVLOG_ERROR, "sdcard sdcard read buff failed");
        close(fd);
        return -1;
    }

    close(fd);
    return ret;
}

int readGraphFromSdCard(char* name, t_graph* graph) {
    int fd = 0;
    int ret = 0;
    struct stat fileStat;
    char fullname[128];
    snprintf(fullname, sizeof(fullname), "/mnt/sdcard/graphs/%s", name);

    // Validate written data
    fd = open(fullname, O_RDONLY);

    if (fd < 0) {
        mvLog(MVLOG_ERROR, "open file %s failed, %d", fullname, fd);
        return -1;
    }

    memset(&fileStat, 0, sizeof(struct stat));
    ret = fstat(fd, &fileStat);
    if (ret < 0 || fileStat.st_size == 0) {
        mvLog(MVLOG_ERROR, "Detect graph file is wrong or size is zero");
        close(fd);
        return -1;
    }
    mvLog(MVLOG_INFO, "Detect Graph File: %s  size: %d", fullname, fileStat.st_size);

    graph->graph = (char*)malloc(fileStat.st_size);

    if (graph->graph == NULL) {
        mvLog(MVLOG_ERROR, "Allocate memory for graph failed!");
        return -1;
    }

    ret = read(fd, graph->graph, fileStat.st_size);

    if (ret > 0) {
        graph->graphLen = fileStat.st_size;

        if (0 == strcmp(name, "face")) {
            graph->type = FACE_TYPE;
        } else if (0 == strcmp(name, "hand")) {
            graph->type = HAND_TYPE;
        } else if (0 == strcmp(name, "object")) {
            graph->type = OBJECT_TYPE;
        } else if (0 ==  strcmp(name, "pose")) {
            graph->type = POSE_TYPE;
        } else if (0 ==  strcmp(name, "carplate")) {
            graph->type = CARPLATE_TYPE;
        } else if (0 ==  strcmp(name, "detect")) {
            graph->type = DETECT_TYPE;
        } else if (0 ==  strcmp(name, "track")) {
            graph->type = TRACK_TYPE;
        } else if (0 ==  strcmp(name, "score")) {
            graph->type = SCORE_TYPE;
        } else if (0 ==  strcmp(name, "attr")) {
            graph->type = ATTR_TYPE;
        } else {
            mvLog(MVLOG_ERROR, "Don't Detect supported model name");
            close(fd);
            return -1;
        }

        mvLog(MVLOG_INFO, "graph size: %d addr: %x type: %d", graph->graphLen, graph->graph, graph->type);
    } else {
        graph->graphLen = 0;
        mvLog(MVLOG_ERROR, "read %s failed", fullname);
        close(fd);
        return -1;
    }

    close(fd);
    return ret;
}

int readAllGraphFromSdCard(void) {
    int fd = 0;
    int ret = 0;
    struct stat fileStat;
    char* graphFile;
    char* name;
    int num = 0;
    memset(lrt_g_graphList, 0, MAX_GRAPH * sizeof(t_graph));

    // Validate written data
    fd = open(MODEL_GRAPHLIST, O_RDONLY);

    if (fd < 0) {
        mvLog(MVLOG_ERROR, "open file %s failed, return %d", MODEL_GRAPHLIST, fd);
        return -1;
    }

    memset(&fileStat, 0, sizeof(struct stat));
    ret = fstat(fd, &fileStat);

    if (ret < 0 || fileStat.st_size == 0) {
        mvLog(MVLOG_ERROR, "graph list file wrong, return %d", ret);
        close(fd);
        return -1;
    }

    graphFile = (char* )malloc(fileStat.st_size);
    if (NULL == graphFile) {
        mvLog(MVLOG_ERROR, "malloc failed no free memory for graph list");
        close(fd);
        return -1;
    }

    ret = read(fd, graphFile, fileStat.st_size);

    if (ret < 0) {
        mvLog(MVLOG_ERROR, "read graphlist failed");
        free(graphFile);
        close(fd);
        return -1;
    }

    name = graphFile;

    for (int i = 0; i < fileStat.st_size; i++) {
        if (NULL != strchr("\r\n \t", name[0])) {
            name++;
            continue;
        }

        if (NULL != strchr("\r\n \t", graphFile[i])) {
            graphFile[i] = 0;
            mvLog(MVLOG_INFO, "Try to Read The %d Model: %s ", i, name);
            ret = readGraphFromSdCard(name, &lrt_g_graphList[num]);
            if (ret <= 0) {
                break;
            }

            num++ ;
            name = &graphFile[i + 1];
        }
    }

    if (ret <= 0) {
        mvLog(MVLOG_ERROR, "load graph failed");
    }

    free(graphFile);
    close(fd);
    return ret;
}