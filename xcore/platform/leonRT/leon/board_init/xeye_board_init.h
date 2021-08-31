/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _XEYE_BOARD_INIT_H
#define _XEYE_BOARD_INIT_H

#include <DrvGpio.h>
#include <stdbool.h>
#include "xeye_info.h"
#include "board_init_bm.h"
#ifdef __cplusplus
extern "C" {
#endif

// extern function
extern bool xeyeBMBoardInit(XeyeBoardInfo_t* XeyeBoardInfo);
extern bool xeyeOSBoardInit(void);

#ifdef __cplusplus
}
#endif
#endif