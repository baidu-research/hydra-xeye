/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _XEYE_WATCHDOG_H
#define _XEYE_WATCHDOG_H

#include <stdio.h>
#include "OsDrvTimerDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

//int XeyeDrvWatchdogInit(uint32_t reset_period_ms);
int XeyeDrvWatchdogInit(uint32_t reset_period_ms, OsDrvWatchDogCallback_t callback);
void XeyeDrvWatchdogFeed(void);
void XeyeDrvWatchdogDisable(void);
#ifdef __cplusplus
}
#endif
#endif
