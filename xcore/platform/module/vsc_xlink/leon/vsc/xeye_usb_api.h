/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _USB_API_H
#define _USB_API_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VSC_STATUS {
    VSC_SUCCESS = 0 ,
    VSC_TIMEOUT = -1,
    VSC_ERROR = -2,
    VSC_NOT_CONNECTED = -3
} VSC_STATUS_t;


int XeyeUsbVscWrite(char* buf, int size, int timeout);
int XeyeUsbVscRead(char* buf, int size, int timeout);
int XeyeUsbVscConnectStatus(void);
int XeyeDrvUsbInit(void);

#ifdef __cplusplus
}
#endif
#endif
