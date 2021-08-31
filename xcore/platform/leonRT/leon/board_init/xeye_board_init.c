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
#include <stdbool.h>
#include <string.h>
#include "xeye_board_init.h"
#include "DrvCDCEL.h"
#include "boardGpioCfg.h"
#include "board_init_bm.h"
#include "xeye_firmware_version.h"
#include "platform_pubdef.h"
#define MVLOG_UNIT_NAME xeye_board_init
#include <mvLog.h>

// xeye board platform 2.0 BM interface
bool xeyeBMBoardInit(XeyeBoardInfo_t* xeyeBoardInfo) {
    int32_t status = 0;
    unsigned int rev;
    BoardInfo brd_info;
    BoardI2CInfo* info;

    mvLog(MVLOG_INFO, "Platform 2.0 Board Init BM start");
    mvLog(MVLOG_INFO, "xeye software version: %s by %s on %s at %s %s \n", get_xeye_version(),\
        get_xeye_compile_by(), get_xeye_compile_host(), get_xeye_compile_date(), get_xeye_compile_time());
    // Board Init Step1: Get board info from eeprom
    status = xeyeGetBrdInfo(&brd_info);

    if (status != BRDCONFIG_SUCCESS) {
        mvLog(MVLOG_ERROR, "Get board info from eeprom error with %ld status", status);
        return false;
    }

    if (strncmp(brd_info.name, BOARD_212_NAME, NUM_CHARS_BRD_NAME) != 0) {
        mvLog(MVLOG_ERROR, "board name %s don't match %s please check eeprom info", brd_info.name,
              BOARD_212_NAME);
    }

    mvLog(MVLOG_INFO, "Board PCB version: %d", brd_info.revision);

    XeyePcbType pcb_version = (XeyePcbType)(brd_info.revision);
    xeyeBoardInfo->hw_pcb_version = pcb_version;
    // Board Init Step2: init gpio and i2c WM8325 according to pcb version
    status = xeyeIneterfaceInit(pcb_version);

    if (status != BRDCONFIG_SUCCESS) {
        mvLog(MVLOG_ERROR, "Platform 2.0 Board --> board init Interface failed with %ld status", status);
        return status;
    }

    // Board Init Step3:  init Extern PLL
    if (pcb_version == XEYE_20) {
        status = xeyeInitExtPll(EXT_PLL_CFG_148_24_24MHZ);

        if (status != BRDCONFIG_SUCCESS) {
            mvLog(MVLOG_ERROR, "Platform 2.0 Board --> board init Ext PLL failed with %ld status", status);
            return -1;
        }
    }

    status = xeyeGetI2CInfo(xeyeBoardInfo->i2c_info, NUM_I2C_DEVS);
    if (status != BRDCONFIG_SUCCESS) {
        mvLog(MVLOG_ERROR, "Error Board --> board get I2C info failed with %ls status!", status);
    }
    xeyeBoardInfo->eeprom_i2c_handle = xeyeBoardInfo->i2c_info[2].handler;
    mvLog(MVLOG_INFO, "Platform 2.0 Board Init BM end");

    // get sensor type here
    XeyeGetSensorType(xeyeBoardInfo);
    return true;
}

// xeye board platform 2.0 Rtems OS interface
bool xeyeOSBoardInit(void) {
    return true;
}

