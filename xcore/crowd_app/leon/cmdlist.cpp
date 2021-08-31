/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

// Includes
// ----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rtems.h>
#include <rtems/bspIo.h>
#include <fcntl.h>
#include "rtems/rtems_bsdnet.h" //rtems_bsdnet_initialize_network()
#include <assert.h>
#include "sys/types.h"
#include <sys/socketvar.h>
#include "sys/socket.h"
#include <netinet/in.h>
#include "netdb.h" //gethostbyname
#include <sys/ioctl.h>
#include "net/if.h"
#include "net/if_var.h"
#include "arpa/inet.h"
#include "sys/proc.h"
#include <bsp/greth_gbit.h>
#include <bsp.h>
#include "DrvGpio.h"
#include "DrvI2cMaster.h"
#include "DrvI2c.h"
#include "OsDrvCpr.h"
#include "OsDrvTimer.h"
#include "rtems/dhcp.h"
#include "time.h"
#include <VcsHooksApi.h>
#include "net_app.h"
#include <rtems/score/todimpl.h>
#include "telnet.h"
#include "cmdlist.h"
#include "spiflash.h"
#include "config.h"
#include "pubdef.h"
#include "spiflash.h"
#include "params/params.hpp"

//
#define PRINT printf
//#define PRINT(...)

extern char  lrt_g_server_addr[128];
extern int lrt_g_server_port;
extern uint8_t lrt_g_ucMac[6];
extern float lrt_g_temp_css;
extern float lrt_g_temp_mss;
extern float lrt_g_temp_upa0;
extern float lrt_g_temp_upa1;
extern char lrt_firm_version[32];
int cmd_status(const char* param, char* resp) {
    int erno = RES_OK;
    char port[128];
    char mac[128];
    char css_temp[32];
    char mss_temp[32];
    char upa0_temp[32];
    char upa1_temp[32];
    assert(resp != NULL && param != NULL);
    snprintf(port,sizeof(port),"%d", Params::getInstance()->data_server_port());
    snprintf(mac,sizeof(port),"%02x%02x-%02x%02x-%02x%02x",
                               lrt_g_ucMac[0],
                               lrt_g_ucMac[1],
                               lrt_g_ucMac[2],
                               lrt_g_ucMac[3],
                               lrt_g_ucMac[4],
                               lrt_g_ucMac[5]);

    snprintf(css_temp, sizeof(css_temp), "%4.4f", lrt_g_temp_css); 
    snprintf(mss_temp, sizeof(mss_temp), "%4.4f", lrt_g_temp_mss); 
    snprintf(upa0_temp, sizeof(upa0_temp), "%4.4f", lrt_g_temp_upa0); 
    snprintf(upa1_temp, sizeof(upa1_temp), "%4.4f", lrt_g_temp_upa1);

    strcat(resp, "mac: ");
    strcat(resp, mac);
    strcat(resp, "\r\n");
    
    strcat(resp, "xeye_id: ");
    strcat(resp, Params::getInstance()->xeye_id().c_str());
    strcat(resp, "\r\n");

    strcat(resp, "temp CSS/MSS/UPA0/UPA1: ");
    strcat(resp, css_temp);
    strcat(resp, ":");
    strcat(resp, mss_temp);
    strcat(resp, ":");
    strcat(resp, upa0_temp);
    strcat(resp, ":");
    strcat(resp, upa1_temp);
    strcat(resp, ":");
    strcat(resp, "\r\n");

    strcat(resp, "ipaddr: ");
    strcat(resp, Params::getInstance()->local_ipaddr().c_str());
    strcat(resp, "\tnetmask: ");
    strcat(resp, Params::getInstance()->local_netmask().c_str());
    strcat(resp, "\tgateway: ");
    strcat(resp, Params::getInstance()->local_gateway().c_str());
    strcat(resp, "\r\ndata_server: ");
    strcat(resp, Params::getInstance()->data_server_addr().c_str());
    strcat(resp, "\tport: ");
    strcat(resp, std::to_string(Params::getInstance()->data_server_port()).c_str());
    strcat(resp, "\r\nntp_server: ");
    strcat(resp, Params::getInstance()->ntp_server_addr().c_str());
    strcat(resp, "\t\tport: ");
    strcat(resp, std::to_string(Params::getInstance()->ntp_server_port()).c_str());
    strcat(resp, "\r\nheartbeat_server: ");
    strcat(resp, Params::getInstance()->heartbeat_server_addr().c_str());
    strcat(resp, "\tport: ");
    strcat(resp, std::to_string(Params::getInstance()->heartbeat_server_port()).c_str());
    strcat(resp, "\r\nplayback_server: ");
    strcat(resp, Params::getInstance()->playback_server_addr().c_str());
    strcat(resp, "\tport: ");
    strcat(resp, std::to_string(Params::getInstance()->playback_server_port()).c_str());
    strcat(resp, "\r\n");

    strcat(resp, "firm_version: ");
    strcat(resp, lrt_firm_version);
    strcat(resp, "\r\n");
    return erno;
}

int cmd_xeye_id(const char* param, char* resp) {
    int erno = RES_OK;
    assert(resp != NULL && param != NULL);
    if (strlen(param) >= 128)
    {
        strcat(resp, "param error.\r\n");
        return erno;
    }

    // this function will automatically update the conf file on disk
    Params::getInstance()->set_xeye_id(std::string(param));

    strcat(resp, "\r\n");

    return erno;
}
int cmd_serveraddr(const char* param, char* resp) {
    int erno = RES_OK;
    char port[128];
    char ip[128];
    assert(resp != NULL && param != NULL);
    if (strlen(param) > strlen("255.255.255.255:800000") || NULL == strstr(param,":"))
    {
        strcat(resp, "param error.\r\n");
        return erno;
    }
    char *tmp = strstr(param,":");

    memset(ip,0,sizeof(ip));
    memset(port,0,sizeof(port));

    memcpy(ip,param,tmp-(char *)param);
    memcpy(port,tmp+1,strlen(param) - strlen(ip)+1);

    // TODO(yanghongtian): add a command to set heartbeat server address and port.
    Params::getInstance()->set_data_server_addr(std::string(ip));
    Params::getInstance()->set_data_server_port(atoi(port));

    strcat(resp, "ip:");
    strcat(resp, ip);
    strcat(resp, "    port:");
    strcat(resp, port);
    strcat(resp, "\r\n");

    return erno;
}


int CMD_help(const char* param, char* resp) {
    int erno = RES_OK;
    assert(resp != NULL && param != NULL);

    for (int i = 0; i < CMD_COUNT; i++) {
        strcat(resp, cmd_table[i].cmdname);
        strcat(resp, "\r\n");
    }

    return erno;
}
int CMD_exit(const char* param, char* resp) {
    int erno = RES_OK;
    assert(resp != NULL && param != NULL);
    PRINT("This is exit command callback\r\n");
    strcpy(resp, "Bye-Bye\r\n");
    return RES_QUIT;
}

int CMD_mkdir(const char* param, char* resp) {
    int erno = RES_OK;
    char folder[CMD_LENGTH] = {0};
    assert(resp != NULL && param != NULL);
    sprintf(folder, "/mnt/sdcard/%s", param);
    erno = mkdir(folder, S_IRWXU | S_IRWXG | S_IRWXO);

    if (erno == 0) {
        strcpy(resp, "Mkdir ok\r\n");
    } else {
        strcpy(resp, "Mkdir fail, folder may already exists\r\n");
    }

    return RES_OK;
}

extern int ReadNetBufferEx(char* buf, int size);
extern int SendTelnetStrEx(char* buf);

extern char* TokenFrontStr(char* strDest, const char* strSrc, u8 token);
extern char* TokenLastStr(char* strDest, const char* strSrc, u8 token);
extern u16 CheckBlankPostion(const char* str, char* dest);
int CMD_upload(const char* param, char* resp) {
    int fd;
    rtems_status_code sc;
    char filename[20] = {0};
    char filetype[20] = {0};
    char filesize[20] = {0};
    char filecheck[20] = {0};
    char filenamepath[40] = {"/mnt/sdcard/"};

    char param_stub[CMD_LENGTH] = {0};
    char tmp[CMD_LENGTH] = {0};
    u32 file_checksum = 0;

    int erno = RES_OK;
    int8_t cfiletype = -1;
    int32_t ifilesize = -1;
    assert(resp != NULL && param != NULL);

    TokenFrontStr(filetype, param, ' ');
    TokenLastStr(param_stub, param, ' ');

    if (!(((memcmp(filetype, "0", 1) == 0) ||
            (memcmp(filetype, "1", 1) == 0) ||
            (memcmp(filetype, "2", 1) == 0)) && (strlen(filetype) == 1))) {
        strcpy(resp, "File type error\r\n");
        return RES_FAIL;
    }

    cfiletype = atoi(filetype);

    CheckBlankPostion(param_stub, tmp);
    TokenFrontStr(filename, tmp, ' ');
    TokenLastStr(param_stub, tmp, ' ');
    PRINT("filename:%s.\r\n", filename);

    CheckBlankPostion(param_stub, tmp);
    TokenFrontStr(filesize, tmp, ' ');
    TokenLastStr(param_stub, tmp, ' ');
    ifilesize = atoi(filesize);
    PRINT("filesize:%d, %s.\r\n", ifilesize, filesize);

    CheckBlankPostion(param_stub, tmp);
    TokenFrontStr(filecheck, tmp, ' ');
    file_checksum = atoi(filecheck);
    PRINT("file checksum:%x, %s.\r\n", file_checksum, filecheck);

    if (ifilesize <= 0 || file_checksum == 0) {
        strcpy(resp,
               "FileSize error or your Client is not support upload file\r\n, Please use xeye client try it again!!!\r\n");
        return RES_FAIL;
    }

    strcpy(resp, "Waiting file upload....\r\n");
    SendTelnetStrEx(resp);

    // Try to create the file if does not exist
    if (cfiletype == FILE_GRAPH) {
        strcat(filenamepath, "graphs/");
    }

    strcat(filenamepath, filename);
    fd = creat(filenamepath, S_IRWXU | S_IRWXG | S_IRWXO) ;
    printf(filenamepath);

    if (fd == 0) {
        goto UPLOAD_FAIL;
    }

    int filebufsize = 1000000;
    char* filebuf = reinterpret_cast<char*>(malloc(filebufsize));

    if (filebuf == NULL) {
        filebufsize = 10000;
        filebuf = reinterpret_cast<char*>(malloc(filebufsize));

        if (filebuf == NULL) {
            goto UPLOAD_FAIL;
        }
    }

    int count = 0;
    int sum = 0;

    while (ifilesize / filebufsize != 0) {
        count = ReadNetBufferEx(filebuf, filebufsize);
        ifilesize -= filebufsize;
        sum += count;
        PRINT("count:%d\r\n", count);
        //fwrite(filebuf, count, 1, fd);
        write(fd, filebuf, count);
    }


    if (ifilesize > 0) {
        count = ReadNetBufferEx(filebuf, ifilesize);
        sum += count;
        //fwrite(filebuf, count, 1, fd);
        write(fd, filebuf, count);
    }

    fsync(fd);
    close(fd);
    PRINT("count:%d, sum:%d\r\n", count, sum);

    FILE* fp;
    fp = fopen(filenamepath, "r");

    if (fp == NULL) {
        PRINT("File open fail\r\n");
        goto UPLOAD_FAIL;
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    PRINT("size:%d\r\n", size);
    fseek(fp, 0, SEEK_SET);
    int8_t readone;
    uint32_t read_checksum = 0;
    for(int i = 0; i < size ; i++) {
        fread(&readone, 1, 1, fp);
        read_checksum+=readone;
    }
    //read_checksum = read_checksum / (size/12745+1);
    if (read_checksum != file_checksum) {
        unlink(filenamepath);
        goto UPLOAD_FAIL;
    }
    if (fp != NULL) {
        fclose(fp);
    }
    free(filebuf);
    strcpy(resp, "OK\r\n");
    return RES_UPLOAD;
UPLOAD_FAIL:
    if (fp != NULL) {
        fclose(fp);
    }
    strcpy(resp, "Fail\r\n");
    return RES_FAIL;
}

extern int SendTelnetDataEx(char* buf, int len);
int CMD_download(const u8* param, u8* resp) {
    int erno = RES_OK;
    assert(resp != NULL && param != NULL);
    FILE* fp;

    u8 filenamepath[40] = {"/mnt/sdcard/"};
    strcat(filenamepath, param);
    fp = fopen(filenamepath, "r");

    if (fp == NULL) {
        PRINT("File open fail\r\n");
        int val = 0;
        SendTelnetDataEx((char*)&val, 4);
        return erno = RES_FAIL;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    PRINT("size:%d\r\n", size);
    fseek(fp, 0, SEEK_SET);

    char* buf = malloc(size);
    if (buf == NULL) {
        int val = 0;
        SendTelnetDataEx((char*)&val, 4);
        fclose(fp);
        return erno = RES_FAIL;
    }
    fread(buf, size, 1, fp);
    SendTelnetDataEx((char*)&size, 4);
    SendTelnetDataEx(buf, size);
    fclose(fp);
    free(buf);

    return erno;
}

int CMD_firmapply(const char* param, char* resp) {
    int erno = RES_OK;
    assert(resp != NULL && param != NULL);
    FILE* fp;

    //u8 filenamepath[40] = {"/mnt/sdcard/"};
    fp = fopen("/mnt/sdcard/firm.bin", "r");

    if (fp == NULL) {
        PRINT("File open fail\r\n");
        erno = RES_FAIL;
        goto FIRMAPPLY_RETURN;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    PRINT("size:%d\r\n", size);
    fseek(fp, 0, SEEK_SET);

    char* buf = malloc(((size / 10240) + 1) * 10240);

    if (buf == NULL) {
        erno = RES_FAIL;
        goto FIRMAPPLY_RETURN;
    }

    memset(buf, 0xFF, ((size / 10240) + 1) * 10240);
    fread(buf, size, 1, fp);
    struct tFlashParams myFlash;
    myFlash.offset = ALIGN_TO_SUBSECTOR(0);
    //myFlash.size = ALIGN_TO_SUBSECTOR(size);
    myFlash.size = ALIGN_TO_SUBSECTOR(((size / 10240) + 1) * 10240);
    printf("erase size:%d\r\n", myFlash.size);
    strcpy(resp, "Start to erase flash....\r\n");
    SendTelnetStrEx(resp);
    erase_spi_flash_Ex(&myFlash);
    strcpy(resp, "Erase flash OK....\r\n");
    SendTelnetStrEx(resp);

    int wsize = 10 * 1024;
    size = ((size / 10240) + 1) * 10240;
    int total_size = size;
    int statis_size = 0;
    int skip = 0;
    myFlash.offset = 0;
    myFlash.inBuff = buf;
    strcpy(resp, "start to write flash...\r\n");
    SendTelnetStrEx(resp);

    while (wsize < size) {
        //myFlash.size = size;
        myFlash.size = wsize;
        write_spi_flash_Ex(&myFlash);
        statis_size += wsize;

        if (skip % 10 == 0) {
            sprintf(resp, "Write %d/%d\r\n", statis_size, total_size);
            SendTelnetStrEx(resp);
        }

        skip++;
        size -= wsize;
        myFlash.offset += wsize;
        myFlash.inBuff += wsize;
    }

    myFlash.size = size;
    write_spi_flash_Ex(&myFlash);
    statis_size += wsize;
    sprintf(resp, "Write %d/%d\r\n", total_size, total_size);
    SendTelnetStrEx(resp);

    strcpy(resp, "Write flash OK....\r\n");
    SendTelnetStrEx(resp);
    free(buf);
FIRMAPPLY_RETURN:

    if (fp != NULL) {
        fclose(fp);
    }
    if (erno == 0) {
        strcpy(resp, "Firmapply successfully\r\n");
    } else {
        strcpy(resp, "Firmapply failed\r\n");
    }

    return RES_OK;
}
int CMD_reset(const char* param, char* resp) {
    int erno = RES_OK;
    assert(resp != NULL && param != NULL);
    strcpy(resp, "system will be reset....\r\n");
    SendTelnetStrEx(resp);
    SET_REG_WORD(CPR_MAS_RESET_ADR, 0x00);
    return erno;
}

const cmdlist_t cmd_table[CMD_COUNT] = {
    {"status", cmd_status},
    {"help", CMD_help},
    {"serveraddr", cmd_serveraddr},
    {"xeye_id", cmd_xeye_id},
    {"exit", CMD_exit},
    {"quit", CMD_exit},
    {"mkdir", CMD_mkdir},
    {"upload", CMD_upload},
    {"download", CMD_download},
    {"firmapply", CMD_firmapply},
    {"reset", CMD_reset},
};

