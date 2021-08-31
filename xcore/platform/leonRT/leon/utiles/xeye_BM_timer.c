/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include <stdbool.h>
#include "mv_types.h"
#include "DrvTimer.h"
#include "xeye_BM_timer.h"
#define MVLOG_UNIT_NAME leonRT_timer
#include <mvLog.h>

// static variable
static tyTimeStamp timer_handler;
static u64 timer_cycles;

int timerStart(void) {
    int ret = 0;

    ret = DrvTimerStartTicksCount(&timer_handler);
    return ret;
}

float getTimeStamp(void) {
    DrvTimerGetElapsedTicks(&timer_handler, &timer_cycles);
    return (float)DrvTimerTicksToMs(timer_cycles);
}

