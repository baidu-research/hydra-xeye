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
#include <stdlib.h>
#include <fcntl.h>
#include <rtems/fsmount.h>
#include <rtems/bdpart.h>
#include "xeye_storage.h"

#include "xeye_sdcard.h"
#include "xeye_emmc.h"
#include "platform_pubdef.h"
#define MVLOG_UNIT_NAME xeye_storage
#include <mvLog.h>


// xeye storage init function
// support sdcard and emmc interface
int xeye_storage_init(bool emmc_enable) {
    int ret = 0;

    if (emmc_enable) {
        ret = XeyeDrvEmmcInit();
        if (ret != 0) {
            mvLog(MVLOG_ERROR, "Init Emmc Failed");
            return -1;
        }
        ret = XeyeGetModelsFromEmmc(1);
        if (ret != 0) {
            mvLog(MVLOG_ERROR, "Emmc Read Graph Failed");
            return -1;
        }
    } else {
        ret = XeyeDrvSdcardInit();
        if (ret != 0) {
            mvLog(MVLOG_ERROR, "Init sdcard Failed");
            return -1;
        }
        readAllGraphFromSdCard();
        if (ret != 0) {
            mvLog(MVLOG_ERROR, "sdcard Read Graph Failed");
            return -1;
        }
    }
    return 0;
}
