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
#include "mvLog.h"
#define PRINT printf
char CmdLine[BUFSIZE] = {0};
char CmdRes[BUFSIZE*100] = {0};
int SockId = 0;

static CmdCallback cmd_callback;
void* telnetd(void* arg) {
    UNUSED(arg);
    int server_sockfd;//
    int client_sockfd;//
    int len;
    struct sockaddr_in server_addr;
    struct sockaddr_in remote_addr;
    socklen_t sin_size;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(23);
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return;
    }
    if (bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0) {
        perror("bind");
        return;
    }
    listen(server_sockfd, 1);
    PRINT("Telnet: listening for telnet requests....\n");
    sin_size = sizeof(struct sockaddr_in);
    while (1) {
        PRINT("start to listen\r\n");
        if ((client_sockfd = accept(server_sockfd, (struct sockaddr*)&remote_addr, &sin_size)) < 0) {
            //perror("accept");
            return;
        } else {
            if (password_verify(client_sockfd) == VERIFY_OK) {
                PRINT("start to commm\r\n");
                SockId = client_sockfd;
                task_process(client_sockfd);
            }
            close(client_sockfd);
        }
    }
}
u32 password_verify(int sockfd) {
    u32 retry_count = 0;
    send_telnet_str(sockfd, "Please input the password:\r\n");
RETRYPASSWD:
    memset(CmdLine, 0, BUFSIZE);
    read_cmd_line(sockfd, CmdLine, BUFSIZE);
    if (strstr(CmdLine, PASSWORD) != NULL) {
        return VERIFY_OK;
    } else {
        send_telnet_str(sockfd, "PassWord error, please try again:\r\n");
        retry_count++;
        if (retry_count > 3) {
            return VERIFY_NG;
        } else {
            goto RETRYPASSWD;
        }
    }
}
void telnet_init(CmdCallback cb) {
    int res;
    pthread_t thread;
    pthread_attr_t attr;
    rtems_greth_gbit_hw_params params;
    cmd_callback = cb;
    if (pthread_attr_init(&attr) != 0) {
        PRINT("pthread_attr_init error\n");
    }
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
        PRINT("pthread_attr_setinheritsched error\n");
    }
    if (pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0) {
        PRINT("pthread_attr_setschedpolicy error\n");
    }
    res = pthread_create(&thread, &attr, &telnetd, NULL);
    if (res) {
        PRINT("Thread 1 creation failed: %d\n", res);
    }
}
int send_telnet_welcome(int fd) {
    return send(fd, "Welcome to Xeye console v1.00\r\n", strlen("Welcome to Xeye console v1.00\r\n"),
                0);
}
int send_telnet_str(int fd, char* buf) {
    return send(fd, buf, strlen(buf), 0);
}
int SendTelnetStr(int fd, char* buf) {
    return send(fd, buf, strlen(buf), 0);
}
int SendTelnetStrEx(char* buf) {
    if (SockId != -1) {
        SendTelnetStr(SockId, buf);
    }
}

int SendTelnetDataEx(char* buf, int len) {
    return send(SockId, buf, len, 0);
}
int read_cmd_line(int sockfd, char* cmdLine, int size) {
    int ret, rev_count = 0;
    char* buf = NULL;
    buf = cmdLine;
    while (1 == (ret = recv(sockfd, buf, 1, 0))) {
        rev_count++;
        if (rev_count > BUFSIZE - 2) {
            return rev_count;
        }
        if (*buf == '\n') {
            return rev_count;
        }
        buf++;
    }
    return ret;
}
int ReadNetBuffer(int sockfd, char* buf, int size) {
    int ret, rev_count = 0;
#if 1

    while (1 == (ret = recv(sockfd, buf, 1, 0))) {
        rev_count++;

        if (rev_count >= size) {
            return rev_count;
        }

        buf++;
    }

#endif
#if 0

    if (1 == (ret = recv(sockfd, buf, size, 0))) {
        return size;
    }

#endif
    return ret;
}
int ReadNetBufferEx(char* buf, int size) {
    return ReadNetBuffer(SockId, buf, size);
}

int cmd_analyze(char* cmd) {
    unsigned char* ptr = NULL;
    unsigned char* ptr_tmp;
    if (strlen(cmd) < 1 || strlen(cmd) > CMD_LENGTH) {
        return -1;
    }
    while ((ptr = strstr(cmd, "\r")) != 0) {
        while (*ptr != 0) {
            *ptr = *(ptr + 1);
            ptr++;
        }
    }
    while ((ptr = strstr(cmd, "\n")) != 0) {
        while (*ptr != 0) {
            *ptr = *(ptr + 1);
            ptr++;
        }
    }
#if 0
    ptr = cmd;
    while ((!((*ptr > 'a' && *ptr < 'z') || (*ptr > 'A' && *ptr < 'Z') || (*ptr > '0' && *ptr < '9')))
            && (*ptr != 0)) {
        ptr_tmp = ptr;
        while (*ptr_tmp != 0) {
            *ptr_tmp = *(ptr_tmp + 1);
            ptr_tmp++;
        }
    }
#endif
    if (strlen(cmd) < 1 || strlen(cmd) > CMD_LENGTH) {
        return -1;
    }
    return 0;
}
void task_process(int sockfd) {
    int count = 0;
    int i = 0, tmp = 0;
    int ret;
    send_telnet_welcome(sockfd);
    while (1) {
        PRINT("\r\r\nXEYE>");
        send_telnet_str(sockfd, "\r\r\nXEYE>");
        memset(CmdLine, 0, sizeof(CmdLine));
        count = read_cmd_line(sockfd, CmdLine, BUFSIZE);
        if (count <= 0) {
            //exit(1);
            return ;
        }
        ret = cmd_analyze(CmdLine);
        PRINT("[%s,%d]rev count:%d,buf:%s\n", __FUNCTION__, __LINE__, count, CmdLine);
        if (ret == 0) {
            memset(CmdRes, 0, BUFSIZE * 100);
            ret = execute_cmd(CmdLine, CmdRes);
            send_telnet_str(sockfd, CmdRes);
            if (ret == RES_QUIT) {
                break;
            }
        }
    }
}
//get fornt str with token
//token_front_str(buf, "192.168.0.34", '.');
//buf value is "192"
int8_t* token_front_str(int8_t* strDest, const int8_t* strSrc, u8 token) {
    assert(strDest != NULL && strSrc != NULL);
    int8_t* pRes;
    pRes = strchr(strSrc, token);
    if (!pRes) {
        strncpy(strDest, strSrc, strlen(strSrc) + 1);
    } else {
        u16 pos = pRes - strSrc;
        strncpy(strDest, strSrc, pos);
        strDest[pos] = '\0';
    }
    return strDest;
}
char* TokenFrontStr(char* strDest, const char* strSrc, u8 token) {
    char* pRes = strchr(strSrc, token);

    if (!pRes) {
        strncpy(strDest, strSrc, strlen(strSrc) + 1);
    } else {
        u16 pos = pRes - strSrc;
        strncpy(strDest, strSrc, pos);
        strDest[pos] = '\0';
    }

    return strDest;
}


int8_t* TokenLastStr(char* strDest, const char* strSrc, u8 token) {
    int8_t* pRes;

    pRes = strchr(strSrc, token);

    if (!pRes) {
        strncpy(strDest, "", 1);
    } else {
        strncpy(strDest, pRes + 1, strlen(pRes) + 1);

    }

    return strDest;
}

u16 CheckBlankPostion(const char* str, char* dest) {
    u8* phead = (u8*)str;
    u8* pend = (u8*)(str + strlen(str));

    while (*phead == ' ' || *phead == 0x09/* \t */) {
        phead ++;
    }

    if (phead == pend) {
        dest[0] = '\0';
        return 0;
    }

    pend --;

    while (*pend == ' ' || *pend == 0x09/* \t */) {
        pend--;
    }

    strncpy((char*)dest, (const char*)phead, (u32)(pend - phead + 1));
    dest[pend - phead + 1] = '\0';
    return 0;
}
//get fornt str with token
//token_last_str(buf, "192.168.0.34", '.');
//buf value is "168.0.34"
int8_t* token_last_str(char* strDest, const char* strSrc, u8 token) {
    assert(strDest != NULL && strSrc != NULL);
    int8_t* pRes;
    pRes = strchr(strSrc, token);
    if (!pRes) {
        strncpy(strDest, "", 1);
    } else {
        strncpy(strDest, pRes + 1, strlen(pRes) + 1);
    }
    return strDest;
}
//delete space int str head and tail
//check_blank_postion("    setval abc 123   ", buf)
//buf value is "setval abc 123"
u16 check_blank_postion(const char* str, char* dest) {
    assert(str != NULL && dest != NULL);
    u8* phead = (u8*)str;
    u8* pend = (u8*)(str + strlen(str));
    while (*phead == ' ' || *phead == 0x09/* \t */) {
        phead ++;
    }
    if (phead == pend) {
        dest[0] = '\0';
        return 0;
    }
    pend --;
    while (*pend == ' ' || *pend == 0x09/* \t */) {
        pend--;
    }
    strncpy((char*)dest, (const char*)phead, (u32)(pend - phead + 1));
    dest[pend - phead + 1] = '\0';
    return 0;
}
int32_t execute_cmd(const u8* str, u8* resp) {
    int ret = 0;
    u8 param[CMD_LENGTH] = {0};
    u8 cmd[CMD_LENGTH] = {0};
    u8 tmp[CMD_LENGTH] = {0};
    int32_t i = 0;
    u8 token = ' ';
    /* command and parameter is split by /0x20 */
    check_blank_postion(str, tmp);
    while ('\0' != tmp[i]) {
        if (tmp[i] == '\t') {
            tmp[i] = ' ';
        }
        i++;
    }
    token_front_str(cmd, tmp, token);
    token_last_str(param, tmp, token);
    strlwr((char*)cmd);
    check_blank_postion(param, tmp);
    strlwr((char*)tmp);
    PRINT("cmd:%s, param:%s\r\n", cmd, tmp);
    i = 0;
    for (; i < CMD_COUNT; i++) {
        if (strcmp((const char*) cmd, cmd_table[i].cmdname) == 0) {
            if (strcmp((const char*)cmd, "firmapply") == 0) {
                if (cmd_callback != nullptr && cmd_callback(0)) {
                    mvLog(MVLOG_INFO, "Suspend deep task successfully");
                }
            }

            if (cmd_table[i].cmdcb != NULL) {
                ret = cmd_table[i].cmdcb(tmp, resp);
            }

            if (strcmp((const char*)cmd, "firmapply") == 0) {
                if (cmd_callback != nullptr && cmd_callback(1)) {
                    mvLog(MVLOG_INFO, "Resume deep task successfully");
                }
            }
            break;
        }
    }
    if (i == CMD_COUNT) {
        strcpy(resp, "Not Support This Command!!!\r\n");
    }
    return ret;
}
