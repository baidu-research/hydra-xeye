/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <rtems/fsmount.h>
#include <rtems/bdpart.h>
#include <VcsHooksApi.h>  // For loadMemFromFileSimple
#include "OsDrvSdioDefines.h"
#include "OsDrvSdio.h"
#include "xeye_emmc.h"
#include "platform_pubdef.h"
#include <mvLog.h>
#include "xeye_md5.h"
#include <bsp.h>
#include <rtems/dosfs.h>
#include <fstream>
#include <DrvCpr.h>
#include <DrvTimer.h>
#include <string.h>
#include <assert.h>
#include <OsDrvTimer.h>
#include "xeye_aes.h"

#define MVLOG_UNIT_NAME xeye_emmc
#define EMMC_DEVNAME_USED   "/dev/sdc0"
#define SLOT_INDEX 1
#define FOLDER "folder"

const std::string ROOT_PATH = "/mnt/sdcard";
const std::string CONFIG = "/mnt/sdcard/CONFIG";
const std::string GRAPHLIST = "/mnt/sdcard/graphlist";
const std::string GRAPHS = "/mnt/sdcard/graphs";

static u8* g_emmc_ptr = (u8*)0xde000000;
static u8* g_emmc_w_ptr = (u8*)0xdf000000;

#define ALIGNED(x) __attribute__((aligned(x)))

ALIGNED(64) unsigned char g_buffer[8 * 1024 * 1024] = {0};
ALIGNED(64) unsigned char config_buffer[16 * 1024] = {0};
ALIGNED(64) unsigned char g_graph_list[MAX_GRAPH][16 * 1024 * 1024] = {0};

extern t_graph lrt_g_graphList[];
static unsigned char graphlist_hash[16] = {0};
static unsigned char models_hash[10][16] = {0};

static FilesHash_t g_files_hash;

int XeyeDrvEmmcInit(void) {
    int sc = 0;
    osDrvSdioEntries_t info = { 1,  // Number of slots
                                10, // Interrupt priority
    {
        {
            1, // Card slot to be used
            EMMC_DEVNAME_USED, // Device name
            SDIO_SDR50, // Max speed mode
            NULL
        }
    }
                              };
    sc = OsDrvSdioInit(&info);

    if (sc != OS_DRV_SDIO_SUCCESS) {
        mvLog(MVLOG_ERROR, "Os drv SDIO Init failed");
        return -1;
    }

    memset((void*)&g_files_hash, 0, sizeof(FilesHash_t));

    XeyeDrvEmmcMountFat32(0);

    return 0;
}

static inline void* XeyeMemToUncachedAddr(void* addr) {
    if ((u32)addr & 0x80000000) {
        addr = (void*)((u32)addr | 0x40000000);
    }

    return addr;
}

int XeyeDrvReadEmmc(u64 from, u32* to, u32 length) {
    DRV_RETURN_TYPE ret = MYR_DRV_SUCCESS;
    u32 flags = 0;
    u32 blockNum = (u32)(from >> 9);

    ret = DrvSdioReadDataBlockEmmc(reinterpret_cast<u32>(to), blockNum, length, SLOT_INDEX, &flags);
    return ret;
}

int XeyeDrvWriteEmmc(u64 to, u32* from, u32 length) {
    DRV_RETURN_TYPE ret = MYR_DRV_SUCCESS;
    u32 flags = 0;
    u32 blockNum = (u32)(to >> 9);

    ret = DrvSdioWriteDataBlockEmmc(blockNum, reinterpret_cast<u32>(from), length, SLOT_INDEX, &flags);
    return ret;
}

u32 XeyeDrvEmmcOpAlignSize(u32 size, u32 align_bit) {
    return ((size + (align_bit - 1)) & (~(align_bit - 1)));
}

int XeyeDrvEmmcClean(int part) {
    int align_size = 0;
    int header_size = 0;
    EmmcHeader_t* emmc_header = NULL;
    header_size = sizeof(EmmcHeader_t);
    align_size = XeyeDrvEmmcOpAlignSize(header_size, BLOCK_SIZE);

    switch (part) {
    case BOOT_PART:
        memset(g_emmc_ptr, 0x0, HEADER_PARTITION);
        XeyeDrvWriteEmmc(BOOT_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);
        break;

    case MODEL_PART:
        XeyeDrvReadEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);
        emmc_header = (EmmcHeader_t*)g_emmc_ptr;
        emmc_header->model_count = 0;
        XeyeDrvWriteEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);
        break;

    case CONFIG_PART:
        XeyeDrvReadEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);
        emmc_header = (EmmcHeader_t*)g_emmc_ptr;
        emmc_header->config_count = 0;
        XeyeDrvWriteEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);
        break;

    case IMAGE_PART:
        XeyeDrvReadEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);
        emmc_header = (EmmcHeader_t*)g_emmc_ptr;
        emmc_header->image_count = 0;
        XeyeDrvWriteEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);
        break;

    case CUSTOM_PART:
        XeyeDrvReadEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);
        emmc_header = (EmmcHeader_t*)g_emmc_ptr;
        emmc_header->custom_count = 0;
        XeyeDrvWriteEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);
        break;

    case HEADER_PART:
    case ALL_PARTS:
        memset(g_emmc_ptr, 0x0, header_size);
        XeyeDrvWriteEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);
        break;

    default:
        break;
    }
}

int XeyeDrvEmmcGetHeader(unsigned char* buf) {
    int align_size = XeyeDrvEmmcOpAlignSize(sizeof(EmmcHeader_t), BLOCK_SIZE);
    //int ret = XeyeDrvReadEmmc(HEADER_PARTITION, g_emmc_ptr, align_size);
    //memcpy(buf, g_emmc_ptr,  sizeof(EmmcHeader_t));
    int ret = XeyeDrvReadEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(buf), align_size);
    return ret;
}

/*
 * Search file in destination partition
 * ret : index
 */
int XeyeDrvEmmcSearchFile(const char* name, int part) {
    EmmcHeader_t* emmc_header = NULL;
    EmmcFile_t* emmc_file = NULL;
    u32 count = 0;
    u32 i = 0;
    int align_size = XeyeDrvEmmcOpAlignSize(sizeof(EmmcHeader_t), BLOCK_SIZE);
    int ret = XeyeDrvReadEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), align_size);

    if (ret == 0) {
        emmc_header = (EmmcHeader_t*)g_emmc_ptr;

        switch (part) {
        case HEADER_PARTITION:
            break;

        case MODEL_PART:
            emmc_file = emmc_header->model;
            count = emmc_header->model_count;
            break;

        case CONFIG_PART:
            emmc_file = emmc_header->config;
            count = emmc_header->config_count;
            break;

        case IMAGE_PART:
            emmc_file = emmc_header->image;
            count = emmc_header->image_count;
            break;

        case CUSTOM_PART:
            emmc_file = emmc_header->custom;
            count = emmc_header->custom_count;
            break;

        default:
            break;
        }

        if (emmc_file) {
            for (i = 0; i < count; i++) {
                if (memcmp(name, emmc_file[i].name, strlen(name)) == 0) {
                    return i;
                }
            }
        }
    }

    return -1;
}

/*
 * Search file in destination partition
 * ret : index
 */
int XeyeDrvEmmcGetFileFromPartition(const char* name, int part, unsigned char* buf, int* len) {
    EmmcHeader_t* emmc_header = NULL;
    EmmcFile_t* emmc_file = NULL;
    u32 count = 0;
    u32 i = 0;
    u8* emmc_ptr = g_buffer;

    int align_size = XeyeDrvEmmcOpAlignSize(sizeof(EmmcHeader_t), BLOCK_SIZE);
    int ret = XeyeDrvReadEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(emmc_ptr), align_size);

    if (ret == 0) {
        emmc_header = (EmmcHeader_t*)emmc_ptr;

        switch (part) {
        case HEADER_PARTITION:
            break;

        case MODEL_PART:
            emmc_file = emmc_header->model;
            count = emmc_header->model_count;
            break;

        case CONFIG_PART:
            emmc_file = emmc_header->config;
            count = emmc_header->config_count;
            break;

        case IMAGE_PART:
            emmc_file = emmc_header->image;
            count = emmc_header->image_count;
            break;

        case CUSTOM_PART:
            emmc_file = emmc_header->custom;
            count = emmc_header->custom_count;
            break;

        default:
            break;
        }

        if (emmc_file) {
            mvLog(MVLOG_INFO, "========>count is %d ", count);

            for (i = 0; i < count; i++) {
                mvLog(MVLOG_INFO, "###%s---%s, %d", name, emmc_file[i].name, strlen(name));

                if (memcmp(name, emmc_file[i].name, strlen(name)) == 0) {
                    align_size = XeyeDrvEmmcOpAlignSize(emmc_file[i].length, BLOCK_SIZE);
                    mvLog(MVLOG_INFO, "name: %s addr: %llu length: %d align_size: %d,  buf addr %x", \
                          emmc_file[i].name, emmc_file[i].addr, emmc_file[i].length, align_size, buf);
                    XeyeDrvReadEmmc(emmc_file[i].addr, reinterpret_cast<u32*>(buf), align_size);
                    *len = emmc_file[i].length;
                    return i;
                }
            }
        }
    }

    return -1;
}

/*
 * Update/Add files in Emmc destination partition
 * Ret: 0 - success, -1 - fail
 */
int EmmcUpdateFiles(char* file_list, int part) {
    int fd = 0;
    int len = 0;
    rtems_status_code sc;
    char folder[128] = {0};
    char o_folder[128] = {0};
    char file[64] = {0};
    char o_file[64] = {0};
    char* str1, *str2, *token, *subtoken;
    char* saveptr1, *saveptr2;
    int j;
    int isFolder = 0;
    int align_size = 0;
    int offset = 0;
    u64 addr = 0;
    u32* count_ptr = NULL;
    EmmcFile_t* emmc_file_ptr = NULL;

    EmmcHeader_t* emmc_header;
    int header_size = XeyeDrvEmmcOpAlignSize(sizeof(EmmcHeader_t), 512);

    if (XeyeDrvReadEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), header_size)) {
        return -1;
    }

    emmc_header = (EmmcHeader_t*)g_emmc_ptr;
    mvLog(MVLOG_INFO, "count model is %d", emmc_header->model_count);

    if (part == MODEL_PART) {
        addr = MODEL_PARTITION;
        count_ptr = &emmc_header->model_count;
        emmc_file_ptr = emmc_header->model;
    } else if (part == CONFIG_PART) {
        addr = CONFIG_PARTITION;
        count_ptr = &emmc_header->config_count;
        emmc_file_ptr = emmc_header->config;
    } else if (part == IMAGE_PART) {
        addr = IMAGE_PARTITION;
        count_ptr = &emmc_header->image_count;
        emmc_file_ptr = emmc_header->image;
    } else if (part == CUSTOM_PART) {
        addr = CUSTOM_PARTITION;
        count_ptr = &emmc_header->custom_count;
        emmc_file_ptr = emmc_header->custom;
    }

    mvLog(MVLOG_INFO, "addr is %llu", addr);

    for (int m = 0; m < emmc_header->model_count; m++) {
        mvLog(MVLOG_INFO, "The m model name %s length: %d addr: %p", \
              emmc_header->model[m].name, emmc_header->model[m].length, emmc_header->model[m].addr);
    }

    mvLog(MVLOG_INFO, "count config is %d", emmc_header->config_count);
    mvLog(MVLOG_INFO, "count image is %d", emmc_header->image_count);
    mvLog(MVLOG_INFO, "count custom is %d", emmc_header->custom_count);

    *count_ptr = 0;

    if (strlen(file_list) > 0) {
        for (j = 1, str1 = file_list; ; j++, str1 = NULL) {
            token = strtok_r(str1, "-", &saveptr1);

            if (token == NULL) {
                break;
            }

            // TODO(zhouqiang): make log message more meaningful
            mvLog(MVLOG_INFO, "%d: %s\n", j, token);

            if (j == 1 && memcmp(token, FOLDER, strlen(FOLDER)) == 0) {
                mvLog(MVLOG_INFO, "Check it is a folder");
                isFolder = 1;
            }

            for (str2 = token; ; str2 = NULL) {
                subtoken = strtok_r(str2, "-", &saveptr2);

                if (subtoken == NULL) {
                    break;
                }

                if (j < 2) {
                    continue;
                }

                // TODO(zhouqiang): make log message more meaningful
                mvLog(MVLOG_INFO, "%d --> %s\n", j, subtoken);

                if (j == 2) {
                    if (isFolder) {
                        sprintf(o_folder, "%s", subtoken);
                    }

                } else if ((j % 2) == 0) {
                    len = atoi(subtoken);
                    // TODO(zhouqiang): make log message more meaningful
                    mvLog(MVLOG_INFO, "===> o_file is %s file is %s", o_file, file);

                    loadMemFromFileSimple(o_file, len, (void*)g_emmc_w_ptr);
                    align_size = XeyeDrvEmmcOpAlignSize(len, BLOCK_SIZE);
                    mvLog(MVLOG_INFO, "len is %d align size is %d", len, align_size);

                    if (XeyeDrvWriteEmmc(addr + offset, reinterpret_cast<u32*>(g_emmc_w_ptr), align_size)) {
                        return -2;
                    }

                    int index = *count_ptr;
                    emmc_file_ptr[index].addr = addr + offset;
                    emmc_file_ptr[index].length = len;
                    memcpy(emmc_file_ptr[index].name, file, strlen(file));
                    emmc_file_ptr[index].name[strlen(file)] = '\0';
                    *count_ptr += 1;
                    offset += align_size;
                } else {
                    sprintf(file, "%s", subtoken);

                    if (isFolder) {
                        sprintf(o_file, "%s/%s", o_folder, subtoken);
                    } else {
                        sprintf(o_file, "%s", subtoken);
                    }

                    // TODO(zhouqiang): make log message more meaningful
                    mvLog(MVLOG_INFO, "file is %s\n", file);
                }
            }
        }
    }

    if (XeyeDrvWriteEmmc(HEADER_PARTITION, reinterpret_cast<u32*>(g_emmc_ptr), header_size)) {
        return -2;
    }

    return 0;
}

char* EmmcPartText(int part) {
    static char* part_str[ALL_PARTS];
    part_str[BOOT_PART] = "boot";
    part_str[HEADER_PART] = "header";
    part_str[MODEL_PART] = "model";
    part_str[CONFIG_PART] = "config";
    part_str[IMAGE_PART] = "image";
    part_str[CUSTOM_PART] = "custom";

    if (part < BOOT_PART || part > ALL_PARTS) {
        return "UNKNOWN";
    } else {
        return part_str[part];
    }
}

int XeyeDrvEmmcPrintInfo(void) {
#define LENGTH (1024 * 1024)
    int i = 0;
    EmmcHeader_t* header_ptr = NULL;
    char buffer[LENGTH];
    mvLog(MVLOG_INFO, "==== 01");

    if (0 != XeyeDrvEmmcGetHeader(g_emmc_ptr)) {
        mvLog(MVLOG_ERROR, "Emmc Read header error!");
        return -1;
    }

    header_ptr = (EmmcHeader_t*)g_emmc_ptr;
    mvLog(MVLOG_INFO, "===============================");
    mvLog(MVLOG_INFO, "Emmc model partition: %d models", header_ptr->model_count);
    mvLog(MVLOG_INFO, "Emmc config partition: %d configs", header_ptr->config_count);
    mvLog(MVLOG_INFO, "Emmc image partition: %d images", header_ptr->image_count);
    mvLog(MVLOG_INFO, "Emmc custom partition: %d custom files", header_ptr->custom_count);

    if (header_ptr->model_count) {
        mvLog(MVLOG_INFO, "====== model partition ======");

        for (i = 0; i < header_ptr->model_count; ++i) {
            mvLog(MVLOG_INFO, "---- %dth %s %dBytes %lluaddr",
                  i + 1,
                  header_ptr->model[i].name,
                  header_ptr->model[i].length,
                  header_ptr->model[i].addr);
        }
    }

    if (header_ptr->config_count) {
        mvLog(MVLOG_INFO, "====== config partition ======");

        for (i = 0; i < header_ptr->config_count; ++i) {
            mvLog(MVLOG_INFO, "---- %dth %s %dBytes %lluaddr",
                  i + 1,
                  header_ptr->config[i].name,
                  header_ptr->config[i].length,
                  header_ptr->config[i].addr);
        }
    }

    if (header_ptr->image_count) {
        mvLog(MVLOG_INFO, "====== image partition ======");

        for (i = 0; i < header_ptr->image_count; ++i) {
            mvLog(MVLOG_INFO, "---- %dth %s %dBytes %lluaddr",
                  i + 1,
                  header_ptr->image[i].name,
                  header_ptr->image[i].length,
                  header_ptr->image[i].addr);
        }
    }

    if (header_ptr->custom_count) {
        mvLog(MVLOG_INFO, "====== custom partition ======");

        for (i = 0; i < header_ptr->custom_count; ++i) {
            mvLog(MVLOG_INFO, "---- %dth %s %dBytes %lluaddr",
                  i + 1,
                  header_ptr->custom[i].name,
                  header_ptr->custom[i].length,
                  header_ptr->custom[i].addr);
        }
    }

    mvLog(MVLOG_INFO, "=========== end ============");
    return 0;
}

// This function need to be refact in the future
int get_file_from_emmc(char* name, int part, char** file_buffer, int* length) {
    int len = 0;

    if (name == NULL) {
        mvLog(MVLOG_ERROR, "Invalid file name!");
        return -1;
    }

    if (part >= ALL_PARTS || part <= HEADER_PART) {
        mvLog(MVLOG_ERROR, "Illegal emmc partition: %d", part);
        return -2;
    }

    if (XeyeDrvEmmcGetFileFromPartition(name, part, g_buffer, &len) < 0) {
        mvLog(MVLOG_ERROR, "Cannot find %s in %d partition!", name, part);
        return -3;
    }

    // *file_buffer = (char*)malloc(len);
    *file_buffer = reinterpret_cast<char*>(g_buffer);

    // if (*file_buffer == NULL) {
    //     mvLog(MVLOG_ERROR, "Malloc %d bytes for %d failed!", len, name);
    //     return -4;
    // }

    *length = len;
    g_buffer[len] = '\0';
    // memcpy((char*)*file_buffer, (char*)config_buffer, len);
    mvLog(MVLOG_DEBUG, "buffer is %s", g_buffer);
    mvLog(MVLOG_DEBUG, "%s content is %s", name, *file_buffer);

    return 0;
}

// brief: get graphlist file from emmc's config partition
// input: none
// output: graphlist -> pointer to memory of graphlist file
//         length -> size of graphlist file
// return: 0-success, others failed

// TODO(hyx): We must read emmc to a buffer for only one time
int get_graphlist_from_emmc(char** graphlist, int* length) {
    int len = 0;

    if (XeyeDrvEmmcGetFileFromPartition("graphlist", CONFIG_PART, config_buffer, &len) < 0) {
        mvLog(MVLOG_ERROR, "Cannot find graphlist in config partition!");
        return -1;
    }

    *graphlist = (char*)malloc(len + 1);

    if (*graphlist == NULL) {
        mvLog(MVLOG_ERROR, "Malloc %d bytes for graphlist failed!", len);
        return -2;
    }

    *length = len;
    config_buffer[len] = '\0';
    memcpy((char*)*graphlist, (char*)config_buffer, len + 1);
    mvLog(MVLOG_DEBUG, "buffer is %s", config_buffer);
    mvLog(MVLOG_DEBUG, "graphlist is %s", *graphlist);

    return 0;
}

// TODO(hyx): We must read emmc to a buffer for only one time
int get_config_file_from_emmc(char* config_file, int* length) {

    int len = 0;
    void* ptr = (void*)XeyeMemToUncachedAddr(config_file);

    if (XeyeDrvEmmcGetFileFromPartition("config", CUSTOM_PART, (unsigned char*)ptr, &len) < 0) {
        mvLog(MVLOG_ERROR, "Cannot find CONFIG in config partition!");
        return -1;
    }

    *length = len;
    config_file[len] = '\0';
    md5((unsigned char*)config_file, len, (unsigned char*)g_files_hash.config_hash);

    return 0;
}

int get_config_file(char* config_file, int* length) {
    assert(config_file);

    std::ifstream ifs(CONFIG);
    if (!ifs.is_open()) {
        mvLog(MVLOG_WARN, "Cannot find %s file!", CONFIG.c_str());
        return -1;
    }
    ifs.seekg(0, ifs.end);
    long filesize = ifs.tellg();
    mvLog(MVLOG_INFO, "%s size is %d\n",CONFIG.c_str(), filesize);
    ifs.seekg(0);
    ifs.read((char*)config_file, filesize);
    ifs.close();

    return 0;
}

// brief: read one graph that matches specific name
// input: name -> the name of graph
// output: graph -> structure of wanted graph
// return: length of graph buffer
int read_graph_from_emmc(char* name, t_graph* graph) {
    int len = 0;

    //TODO(zhoury): Refactor model macth
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
    } else if (0 ==  strcmp(name, "align")) {
        graph->type = ALIGN_TYPE;
    } else if (0 ==  strcmp(name, "attr")) {
        graph->type = ATTR_TYPE;
    } else {
        mvLog(MVLOG_ERROR, "Unsupported model file: %s", name);
        return -1;
    }

    unsigned char* graph_ptr = reinterpret_cast<unsigned char*>(graph->graph);

    if (XeyeDrvEmmcGetFileFromPartition(name, MODEL_PART, graph_ptr, &len) < 0) {
        mvLog(MVLOG_ERROR, "Get model %s failed!", name);
        return -2;
    }

    mvLog(MVLOG_DEBUG, "model name is: %s, length is %d, buffer length is %d", name, strlen(name), len);
    graph->graphLen = len;

    return len;
}

// brief: read all graphs in emmc model partition
// input: none
// output: none
// return: 0-success, others failed
int read_all_graph_from_emmc(void) {
    int sc = 0;
    int graphlist_len = 0;
    int num = 0;
    char* graphlist_ptr = NULL;
    char* name = NULL;
    // t_graph graph_list[MAX_GRAPH];

    for (int i = 0; i < MAX_GRAPH; ++i) {
        // graph_list[i].graph = g_graph_list[i];
        lrt_g_graphList[i].graph = reinterpret_cast<char*>(g_graph_list[i]);
    }

    sc = get_graphlist_from_emmc(&graphlist_ptr, &graphlist_len);

    if (sc) {
        mvLog(MVLOG_ERROR, "Failed to get graphlist from emmc!");
        return -1;
    }

    md5((unsigned char*)graphlist_ptr, graphlist_len, (unsigned char*)g_files_hash.graphlist_hash);
    name = graphlist_ptr;
    mvLog(MVLOG_INFO, "graphlist content is: %s, length is: %d", graphlist_ptr, graphlist_len);

    for (int i = 0; i < graphlist_len; ++i) {
        if (NULL != strchr("\r\n \t", name[0])) {
            name++;
            continue;
        }

        if (NULL != strchr("\r\n \t", graphlist_ptr[i])) {
            graphlist_ptr[i] = 0;
            mvLog(MVLOG_INFO, "model name: %s", name);
            mvLog(MVLOG_INFO, "current graphlist content is: %s, i = %d", graphlist_ptr, i);
            sc = read_graph_from_emmc(name, &lrt_g_graphList[num]);

            if (sc <= 0) {
                break;
            }

            memcpy(g_files_hash.models_name[num], name, strlen(name) + 1);
            g_files_hash.models_num = num + 1;

            if (++num > MAX_GRAPH - 1) {
                break;
            }

            name = &graphlist_ptr[i + 1];
        }
    }

    if (sc <= 0) {
        mvLog(MVLOG_ERROR, "load graph failed");
        return -2;
    }

    // TODO(hyx): Cannot free graphlist_ptr ?
    // if (graphlist_ptr) {
    //     free(graphlist_ptr);
    // }
    return 0;
}

// @brief: read all models appeared in GRAPHLIST file with/without encryption
// @input: encrypt, 0 indicates without encryption, 1 indicates with encryption
// @return: 0 success, others fail
int XeyeGetModelsFromEmmc(int encrypt) {
    u8 read_buffer[2048] = {0};
    int model_index = 0;
    std::ifstream ifs;
    ifs.open(GRAPHLIST);
    ifs.seekg(0, ifs.end);
    long filesize = ifs.tellg();
    mvLog(MVLOG_INFO, "%s size is %d\n", GRAPHLIST.c_str(), filesize);
    ifs.seekg(0);
    memset(read_buffer, 0, 2048);
    ifs.read((char*)read_buffer, filesize);
    ifs.close();

    const char* key_content = "ola&des9pac9ito6";
    uint8_t* key = (uint8_t*) key_content;
    //aes_cbc_crypt((char*)read_buffer, 0, key, filesize);
    //saveMemoryToFile((u32)read_buffer, filesize, "graphlist-dec");

    read_buffer[filesize] = '\0';
    mvLog(MVLOG_INFO, "buf is %s, file size is %d\n", read_buffer, filesize);
    md5((unsigned char*)read_buffer, filesize, (unsigned char*)g_files_hash.graphlist_hash);
    char* name = (char*)read_buffer;

    for (int i = 0; i < filesize; i++) {
        if (NULL != strchr("\r\n \t", *name)) {
            name++;
            continue;
        }

        if (NULL != strchr("\r\n \t", read_buffer[i])) {
            read_buffer[i] = 0;
            std::string model_name(name);
            std::string model_path = GRAPHS + "/" + model_name;
            ifs.open(model_path);
            ifs.seekg(0, ifs.end);
            int len = ifs.tellg();
            ifs.seekg(0);
            ifs.read((char*)g_graph_list[model_index], len);
            lrt_g_graphList[model_index].graph = (char*)g_graph_list[model_index];
            if (encrypt != 0) {
                aes_cbc_crypt((char*)lrt_g_graphList[model_index].graph, 0, key, len);
            }
            memcpy(g_files_hash.models_name[model_index], name, strlen(name) + 1);

            // saveMemoryToFile((u32)lrt_g_graphList[model_index].graph, len, name);
            if (0 == strcmp(name, "face")) {
                lrt_g_graphList[model_index].type = FACE_TYPE;
            } else if (0 == strcmp(name, "hand")) {
                lrt_g_graphList[model_index].type = HAND_TYPE;
            } else if (0 == strcmp(name, "object")) {
                lrt_g_graphList[model_index].type = OBJECT_TYPE;
            } else if (0 ==  strcmp(name, "pose")) {
                lrt_g_graphList[model_index].type = POSE_TYPE;
            } else if (0 ==  strcmp(name, "carplate")) {
                lrt_g_graphList[model_index].type = CARPLATE_TYPE;
            } else if (0 ==  strcmp(name, "detect")) {
                lrt_g_graphList[model_index].type = DETECT_TYPE;
            } else if (0 ==  strcmp(name, "track")) {
                lrt_g_graphList[model_index].type = TRACK_TYPE;
            } else if (0 ==  strcmp(name, "score")) {
                lrt_g_graphList[model_index].type = SCORE_TYPE;
            } else if (0 ==  strcmp(name, "align")) {
                lrt_g_graphList[model_index].type = ALIGN_TYPE;
            } else if (0 ==  strcmp(name, "attr")) {
                lrt_g_graphList[model_index].type = ATTR_TYPE;
            } else {
                mvLog(MVLOG_ERROR, "Unsupported model file: %s", name);
                return -1;
            }

            lrt_g_graphList[model_index].graphLen = len;
            model_index ++;
            ifs.close();
            name = (char*)&read_buffer[i + 1];
        }
    }

    mvLog(MVLOG_INFO, "loading model ready!");
    g_files_hash.models_num = model_index;
    return 0;
}

FilesHash_t* get_emmc_files_hash(void) {
    int i = 0;

    for (i = 0; i < g_files_hash.models_num; i++) {
        md5((unsigned char*)lrt_g_graphList[i].graph, lrt_g_graphList[i].graphLen,
            g_files_hash.models_hash[i]);
    }

    return &g_files_hash;
}

int XeyeUpdateModelAndConfig(void) {
    int ret = 0;
#ifdef UPDATE_MEM
    UpdateFileListHeader_t* header = (UpdateFileListHeader_t*)UPDATE_MEM;
    int i = 0;

    if (header->magic == EMMC_MAGIC) {
        mvLog(MVLOG_INFO, "start updating models/config, file num : %d......\n", header->count);

        for (i = 0; i < header->count; i++) {
            mvLog(MVLOG_INFO, "file name %s, addr %x, type %d, fresh %d\n", header->list[i].name, \
                  header->list[i].addr, header->list[i].type, header->list[i].fresh);
            ret = EmmcUpdateFileFromMemory((void*)header->list[i].addr, header->list[i].name, \
                                           header->list[i].len, header->list[i].type, header->list[i].fresh);
        }

        header->magic = 0;
        mvLog(MVLOG_INFO, "finish updating!\n");
        sleep(1);
        SET_REG_WORD(CPR_MAS_RESET_ADR, 0x00);
    }

#endif
    return ret;
}

int EmmcUpdateFileFromMemory(void* mem, const char* name, u32 len, int part, int fresh) {
    rtems_status_code sc;
    u32 align_size = 0;
    u32 offset = 0;
    u64 addr = 0;
    u32 count = 0;
    u32 index = 0;
    EmmcFile_t* emmc_file_ptr = NULL;
    EmmcHeader_t* emmc_header;
    g_emmc_ptr = g_buffer;

    if (memcmp(name, "graphlist", strlen("graphlist")) == 0) {
        XeyeDrvEmmcClean(ALL_PARTS);
    }

    int header_size = XeyeDrvEmmcOpAlignSize(sizeof(EmmcHeader_t), 512);

    if (XeyeDrvReadEmmc(HEADER_PARTITION, (u32*)g_emmc_ptr, header_size)) {
        return -1;
    }

    emmc_header = (EmmcHeader_t*)g_emmc_ptr;
    printf("count model is %d\n", emmc_header->model_count);

    if (part == MODEL_PART) {
        addr = MODEL_PARTITION;
        count = emmc_header->model_count;
        emmc_file_ptr = emmc_header->model;
    } else if (part == CONFIG_PART) {
        addr = CONFIG_PARTITION;
        count = emmc_header->config_count;
        emmc_file_ptr = emmc_header->config;
    } else if (part == IMAGE_PART) {
        addr = IMAGE_PARTITION;
        count = emmc_header->image_count;
        emmc_file_ptr = emmc_header->image;
    } else if (part == CUSTOM_PART) {
        addr = CUSTOM_PARTITION;
        count  = emmc_header->custom_count;
        emmc_file_ptr = emmc_header->custom;
    }

    if (fresh) {
        index = 0;
        count = 0;
    } else {
        index = count;
        addr = emmc_file_ptr[index - 1].addr + XeyeDrvEmmcOpAlignSize(emmc_file_ptr[index - 1].length, \
                BLOCK_SIZE);
    }

    align_size = XeyeDrvEmmcOpAlignSize(len, BLOCK_SIZE);
    mvLog(MVLOG_INFO, "file in emmc addr is %llu, len %d, align size is %d, partition is %d\n", addr,
          len, align_size, part);

    g_emmc_w_ptr = (u8*)XeyeMemToUncachedAddr((void*)mem);

    if (XeyeDrvWriteEmmc(addr, (u32*)g_emmc_w_ptr, align_size)) {
        return -2;
    }

    emmc_file_ptr[index].addr = addr;
    emmc_file_ptr[index].length = len;
    memcpy(emmc_file_ptr[index].name, name, strlen(name));
    emmc_file_ptr[index].name[strlen(name)] = '\0';

    if (part == MODEL_PART) {
        emmc_header->model_count = fresh ? 1 : count + 1;
    } else if (part == CONFIG_PART) {
        emmc_header->config_count = fresh ? 1 : count + 1;
    } else if (part == IMAGE_PART) {
        emmc_header->image_count = fresh ? 1 : count + 1;
    } else if (part == CUSTOM_PART) {
        emmc_header->custom_count = fresh ? 1 : count + 1;
    }

    mvLog(MVLOG_INFO,
          "emmc: file updated is %s, model count %d, config count %d, image count %d, custom count %d\n", \
          emmc_file_ptr[index].name, emmc_header->model_count, emmc_header->config_count, \
          emmc_header->image_count, emmc_header->custom_count);

    if (XeyeDrvWriteEmmc(HEADER_PARTITION, (u32*)g_emmc_ptr, header_size)) {
        return -2;
    }

    return 0;
}

#define NB_OF_PARTITIONS 1
static const rtems_fstab_entry fs_table [] = {
    {
        .source = "/dev/sdc0",
        .target = ROOT_PATH.c_str(),
        .type = "dosfs",
        .options = RTEMS_FILESYSTEM_READ_WRITE,
        .report_reasons = RTEMS_FSTAB_NONE,
        .abort_reasons = RTEMS_FSTAB_OK
    },
    {
        .source = "/dev/sdc01",
        .target = ROOT_PATH.c_str(),
        .type = "dosfs",
        .options = RTEMS_FILESYSTEM_READ_WRITE,
        .report_reasons = RTEMS_FSTAB_NONE,
        .abort_reasons = RTEMS_FSTAB_NONE
    }
};
static rtems_bdpart_partition created_partitions [RTEMS_BDPART_PARTITION_NUMBER_HINT];
static const unsigned distribution [NB_OF_PARTITIONS] = {1};
static const msdos_format_request_param_t rqdata = {
    /* Optimized for read/write speed */
    .OEMName             = NULL,
    .VolLabel            = NULL,
    .sectors_per_cluster = 64,
    .fat_num             = 0,
    .files_per_root_dir  = 0,
    .media               = 0,
    .quick_format        = true,
    .skip_alignment      = false,
    .sync_device         = false
};

static const rtems_bdpart_format requested_format = {
    .mbr = {
        .type = RTEMS_BDPART_FORMAT_MBR,
        .disk_id = 0xdeadbeef,
        .dos_compatibility = true
    }
};

static void create_partition(const char* disk, uint32_t partitions) {
    uint32_t i;
    rtems_status_code status;

    // Initialize partitions types
    for (i = 0; i < partitions; i ++) {
        rtems_bdpart_to_partition_type(RTEMS_BDPART_MBR_FAT_32, created_partitions[i].type);
    }

    // Create 1 partition on sdc0
    status = rtems_bdpart_create(disk, &requested_format, &created_partitions[0], &distribution [0],
                                 partitions);
    assert(status == 0);
    //physically write the partition down to media
    status = rtems_bdpart_write(disk, &requested_format, &created_partitions[0], partitions);
    assert(status == 0);
}

int XeyeDrvEmmcMountFat32(int format) {

    int sc = 0;
    create_partition("/dev/sdc0", NB_OF_PARTITIONS);

    sc = rtems_bdpart_register_from_disk(EMMC_DEVNAME_USED);
    printf("\nrtems_bdpart_register_from_disk sc %s \n", rtems_status_text((rtems_status_code)sc));

    if (format) {
        int status = msdos_format("/dev/sdc01", &rqdata);
        printf("\nFormatted the SD card (status = %d) \n", status);
        assert(status == 0);
    }

    sc = rtems_fsmount(fs_table, sizeof(fs_table) / sizeof(fs_table[0]), NULL);
    printf("\nMounting File System %s \n", rtems_status_text((rtems_status_code)sc));
    return sc;
}


