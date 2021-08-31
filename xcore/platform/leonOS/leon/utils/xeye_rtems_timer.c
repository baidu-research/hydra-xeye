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
#include <rtems.h>
#include <rtems/score/todimpl.h>
#include "mv_types.h"
#include "OsDrvTimer.h"
#include "xeye_rtems_timer.h"
#define MVLOG_UNIT_NAME leonOS_timer
#include <mvLog.h>

uint64_t get_current_time(void) {
    Timestamp_Control time;
    uint64_t secs = 0;
    uint32_t usecs = 0;
    uint64_t msecs = 0;

    _TOD_Get(&time);
    secs = _Timestamp_Get_seconds(&time);
    usecs = _Timestamp_Get_nanoseconds(&time);
    msecs = (secs * 1000) + (usecs / 1000000);
    return msecs;
}