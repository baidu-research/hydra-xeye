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
#ifndef _CMDLIST_H_
#define _CMDLIST_H_

#define RES_OK      0
#define RES_QUIT    -1
#define RES_FAIL    -2
#define RES_UPLOAD  3

#define FILE_FIRM    0
#define FILE_GRAPH   1
#define FILE_CONFIG  2

typedef int (*CMD)(const char* param, char* resp);

typedef struct _cmdlist_t_ {
    char cmdname[CMD_LENGTH];
    const CMD cmdcb;
} cmdlist_t;

extern const cmdlist_t cmd_table[CMD_COUNT];
extern int CMD_help(const char* param, char* resp);
extern int CMD_exit(const char* param, char* resp);
extern int CMD_upload(const char* param, char* resp);
extern int CMD_mkdir(const char* param, char* resp);
extern int CMD_firmapply(const char* param, char* resp);
extern int CMD_reset(const char* param, char* resp);

//extern global value
//los
extern volatile uint32_t __attribute__((section(" .ddr_direct.data"))) g_uiSendFrameCount;
extern volatile int32_t __attribute__((section(" .ddr_direct.data"))) g_iPackErrorCnt;
extern volatile u8 __attribute__((section(" .ddr_direct.bss"))) g_ucDetectModelStatus;
extern volatile u8 __attribute__((section(" .ddr_direct.bss"))) g_ucObjectModelStatus;
extern uint32_t g_uiGsensorNetPackCnt;
extern uint32_t g_uiGsensorNetSendCnt;
extern uint32_t g_uiGsensorSendFrameCount;
extern volatile int32_t __attribute__((section(" .ddr_direct.data"))) g_iGsensorPackErrorCnt;

//lrt
extern uint8_t __attribute__((section(" .ddr_direct.data"))) lrt_g_enable_usb_display;
extern volatile int __attribute__((section(" .ddr_direct.data"))) lrt_g_iTriggerCount;
extern uint8_t __attribute__((section(".cmx_direct.data"))) lrt_g_ucMac[6];
extern volatile __attribute__((section(".cmx_direct.data"))) u32 lrt_newCamFrameCtr;
extern volatile __attribute__((section(".cmx_direct.data"))) u32 lrt_LcdFrameCtr;
extern volatile u8 __attribute__((section(" .ddr_direct.bss"))) lrt_g_ucJpegEncodeStatus;
extern volatile u8 __attribute__((section(" .ddr_direct.bss"))) lrt_g_ucYuvSippStatus;
extern volatile int __attribute__((section(".ddr_direct.data"))) lrt_g_aec_flag;
extern volatile u8 __attribute__((section(" .ddr_direct.data"))) lrt_g_ucDeepEnableFlag;
extern volatile uint32_t __attribute__((section(".ddr_direct.data"))) lrt_g_uiRtWaittime;

#endif

