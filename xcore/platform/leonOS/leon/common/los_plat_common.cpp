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
#include <string.h>
#include <stdio.h>
#include "mv_types.h"
#include "xeye_uart.h"
#include "temperature.h"
#include "los_plat_common.h"
#define MVLOG_UNIT_NAME xeye_los_common
#include <mvLog.h>

// Source Specific #defines and types  (typedef,enum,struct)

// Functions Implementation
bool leonOS_platform_init(LosSysConfig_t sys_config) {
    s32 sc = 0;

    // init clock
    if (sys_config.sys_clock_enable) {
        sc = initClocks(sys_config.sys_clock_Hz);
        if (sc) {
            return false;
            mvLog(MVLOG_ERROR, "Init Leon OS Clock Fail!");
        }
    }
    // init memory
    if (sys_config.sys_memory_enable) {
        sc = initMemory(sys_config.mem_policy);
        if (sc) {
            return false;
            mvLog(MVLOG_ERROR, "Init Leon OS Memory Fail!");
        }
    }
    if (sys_config.uart_enable) {
        // TODO(hyx): use os interface
        xeye_drv_uart_init(115200);
        printf("xeye platform init uart module\n");
    }
    if (sys_config.temperature_enable) {
        TemperatureInit();
        printf("xeye platform init temperature module\n");
    }
    return true;
}

