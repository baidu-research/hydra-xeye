/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _TELNET_H_
#define _TELNET_H_

#include <functional>
typedef std::function<bool(int)> CmdCallback;

#define VERIFY_OK       1
#define VERIFY_NG       0
#define PASSWORD        "xeye010OK"
#define PASSWD_LENGTH   9
#define CMD_COUNT       11
#define CMD_LENGTH      48
#define BUFSIZE         1024
typedef int SOCKET;
extern void* telnetd(void* args);
extern u32 password_verify(int sockfd);
extern void telnet_init(CmdCallback cb);
extern int send_telnet_welcome(int fd);
extern int send_telnet_str(int fd, char* buf);
extern int read_cmd_line(int sockfd, char* cmdLine, int size);
extern int cmd_analyze(char* cmd);
extern int cmd_process(int fd, char* cmdLine);
extern void task_process(int sockfd);
extern int8_t* token_front_str(int8_t* strDest, const int8_t* strSrc, u8 token);
extern int8_t* token_last_str(char* strDest, const char* strSrc, u8 token);
extern u16 check_blank_postion(const char* str, char* dest);
extern int32_t execute_cmd(const u8* str, u8* resp);

#endif
