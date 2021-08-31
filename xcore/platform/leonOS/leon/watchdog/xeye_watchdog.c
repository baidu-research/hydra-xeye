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
#include "OsDrvTimer.h"
#define MVLOG_UNIT_NAME xeye_wd
#include <mvLog.h>

static u32 fead_watchdog_time = 0;
int XeyeDrvWatchdogInit(uint32_t reset_period_ms, OsDrvWatchDogCallback_t callback) {
    int ret;
    OsDrvWatchDogCfg_t cfg;

    cfg.callbackFunc = callback;
    cfg.callbackPriority = 12;
    cfg.countdownSysRstMs = reset_period_ms;
    cfg.countdownThresholdMs = 200;
    fead_watchdog_time = reset_period_ms;
    OsDrvTimerInit();
    OsDrvTimerWatchdogEnable(WDOG_DISABLE);
    ret = OsDrvTimerWatchdogConfigure(&cfg);

    if (ret == 0) {
        mvLog(MVLOG_INFO, "Watchdog --->Init Successfully!\n");
        OsDrvTimerWatchdogEnable(WDOG_ENABLE);
    } else {
        mvLog(MVLOG_ERROR, "Watchdog Error--->Init failed!\n");
    }

    return ret;
}

void XeyeDrvWatchdogFeed(void) {
    u32 sysRstMs = fead_watchdog_time;
    u32 remainingMs;
    OsDrvTimerWatchdogUpdate(sysRstMs, &remainingMs);
}

void XeyeDrvWatchdogDisable(void) {
    OsDrvTimerWatchdogEnable(WDOG_DISABLE);
}

