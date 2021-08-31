/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include <registersMyriad.h>
#include <DrvTimer.h>
#include <DrvCpr.h>
#include <DrvSvu.h>
#include <DrvCmxDma.h>
#include <DrvRegUtils.h>
#include <DrvShaveL2Cache.h>
#include <DrvDdr.h>
#include <DrvLeonL2C.h>
#include "swcFrameTypes.h"
#include <swcShaveLoader.h>
#include "lrt_sys_config.h"

// Source Specific #defines and types  (typedef,enum,struct)
#define SYS_CLK_KHZ 12000 // 12MHz

// Global Data (Only if absolutely necessary)
// Static Local Data
// Static Function Prototypes
// Functions Implementation
// TODO(wangxiang & renyi) we need add low power control for leonRT here
int init_lrt_sys_clk(void) {
    s32 sc;

    sc = DrvCprInitClocks(SYS_CLK_KHZ, 0, 0, 0, 0);
    if (sc) {
        return sc;
    }

    sc = DrvTimerInit();
    if (sc) {
        return sc;
    }

    return 0;
}
