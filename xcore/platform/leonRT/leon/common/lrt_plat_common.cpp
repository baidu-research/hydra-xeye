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
#include "lrt_plat_common.h"
#include "lrt_sys_config.h"
#include "xeye_board_init.h"
#define MVLOG_UNIT_NAME xeye_lrt_common
#include <mvLog.h>

// Source Specific #defines and types  (typedef,enum,struct)

// Functions Implementation
bool leonRT_platform_init(const LrtSysConfig_t sys_config, XeyeBoardInfo_t* xeye_board_info) {
    if (sys_config.sys_clock_enable) {
        s32 status = init_lrt_sys_clk();
        if (status) {
            mvLog(MVLOG_ERROR, "init Clock and Memory failed");
        }
    }
    if (sys_config.board_init_enable) {
        xeyeBMBoardInit(xeye_board_info);
    }
    timerStart();
    return true;
}

