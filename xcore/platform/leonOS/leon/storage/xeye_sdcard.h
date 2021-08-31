/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _XEYE_SDCARD_H
#define _XEYE_SDCARD_H

#ifdef __cplusplus
extern "C" {
#endif

int XeyeDrvSdcardInit(void);
int readAllGraphFromSdCard(void);
int readFileFromSdCard(char* fileName, char* szBuffer, int iLen);

#ifdef __cplusplus
}
#endif
#endif
