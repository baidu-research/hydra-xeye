/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "DrvDdr.h"
#include "DrvShaveL2Cache.h"
#include <registersMyriad.h>
#include <DrvTimer.h>
#include <DrvCpr.h>
#include <DrvSvu.h>
#include <DrvCmxDma.h>
#include <DrvRegUtils.h>
#include <DrvLeonL2C.h>
#include "swcFrameTypes.h"
#include <swcShaveLoader.h>
#include "OsDrvTimer.h"
#include "OsDrvCpr.h"
#include <OsDrvSvu.h>
#include <OsDrvCmxDma.h>
#include <OsDrvShaveL2Cache.h>
#include <myriad2_version.h>
#include "los_sys_config.h"
#define MVLOG_UNIT_NAME los_sys_config
#include <mvLog.h>
// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define CMX_CONFIG_SLICE_7_0       (0x11111111)
#define CMX_CONFIG_SLICE_15_8      (0x11111111)
#define L2CACHE_CFG                (SHAVE_L2CACHE_NORMAL_MODE)
#define PARTITION_0                (0)
#define SHAVES_USED                (12)

#define SYS_CLK_KHZ 12000 // 12MHz
#define MAX_SHAVES 12

#define MSS_CLOCKS      ( DEV_MSS_APB_SLV     |     \
                          DEV_MSS_APB2_CTRL   |     \
                          DEV_MSS_RTBRIDGE    |     \
                          DEV_MSS_RTAHB_CTRL  |     \
                          DEV_MSS_LRT         |     \
                          DEV_MSS_LRT_DSU     |     \
                          DEV_MSS_LRT_L2C     |     \
                          DEV_MSS_LRT_ICB     |     \
                          DEV_MSS_AXI_BRIDGE  |     \
                          DEV_MSS_MXI_CTRL    |     \
                          DEV_MSS_MXI_DEFSLV  |     \
                          DEV_MSS_AXI_MON     |     \
                          DEV_MSS_MIPI        |     \
                          DEV_MSS_AMC         |     \
                          DEV_MSS_SIPP        |     \
                          DEV_MSS_TIM         )

#define APP_SIPP_CLOCKS ( DEV_SIPP_SIPP_ABPSLV | \
                          DEV_SIPP_APB_SLV     | \
                          DEV_SIPP_MIPI        | \
                          DEV_SIPP_LUT         | \
                          DEV_SIPP_MIPI_RX1    )
#define APP_UPA_CLOCKS (DEV_UPA_SH0       | \
                        DEV_UPA_SH1       | \
                        DEV_UPA_SH2       | \
                        DEV_UPA_SH3       | \
                        DEV_UPA_SH4       | \
                        DEV_UPA_SH5       | \
                        DEV_UPA_SH6       | \
                        DEV_UPA_SH7       | \
                        DEV_UPA_SH8       | \
                        DEV_UPA_SH9       | \
                        DEV_UPA_SH10      | \
                        DEV_UPA_SH11      | \
                        DEV_UPA_SHAVE_L2  | \
                        DEV_UPA_CDMA      | \
                        DEV_UPA_CTRL      )

#define EXTRACLOCKS   (DEV_MSS_APB_SLV     | \
                       DEV_MSS_APB2_CTRL   | \
                       DEV_MSS_RTBRIDGE    | \
                       DEV_MSS_RTAHB_CTRL  | \
                       DEV_MSS_LRT         | \
                       DEV_MSS_LRT_DSU     | \
                       DEV_MSS_LRT_L2C     | \
                       DEV_MSS_LRT_ICB     | \
                       DEV_MSS_AXI_BRIDGE  | \
                       DEV_MSS_MXI_CTRL  )



#define CLOCKS_MIPICFG (AUX_CLK_MASK_MIPI_ECFG | AUX_CLK_MASK_MIPI_CFG)

// PLL desired frequency
#define PLL_DESIRED_FREQ_KHZ        600000
// Default start up clock
#define DEFAULT_OSC0_KHZ            12000
// we only use 600M system clock for all app
BSP_SET_CLOCK(DEFAULT_OSC0_KHZ, PLL_DESIRED_FREQ_KHZ, 1, 1, DEFAULT_CORE_CSS_DSS_CLOCKS,  MSS_CLOCKS,  APP_UPA_CLOCKS,
              APP_SIPP_CLOCKS, 0x0);
// Set the L2C at startup
BSP_SET_L2C_CONFIG(1, L2C_REPL_LRU, 0, 2, 0, NULL);
// 3: Global Data (Only if absolutely necessary)
// Sections decoration is require here for downstream tools
// 4: Static Local Data

static tyAuxClkDividerCfg auxClk[] = {
    {
        .auxClockEnableMask     = AUX_CLK_MASK_MEDIA,         // SIPP Clock
        .auxClockSource         = CLK_SRC_SYS_CLK_DIV2,
        .auxClockDivNumerator   = 1,
        .auxClockDivDenominator = 1,
    },
    {
        .auxClockEnableMask     = AUX_CLK_MASK_CIF0,  // CIFs Clock
        .auxClockSource         = CLK_SRC_SYS_CLK,
        .auxClockDivNumerator   = 1,
        .auxClockDivDenominator = 1,
    },
    {
        .auxClockEnableMask     = CLOCKS_MIPICFG,             // MIPI CFG + ECFG Clock
        .auxClockSource         = CLK_SRC_SYS_CLK,
        .auxClockDivNumerator   = 1,
        .auxClockDivDenominator = 24,                         // the MIPI cfg clock should be <= 20 Mhz !
    },
    {
        .auxClockEnableMask     = AUX_CLK_MASK_USB_CTRL_SUSPEND_CLK,
        .auxClockSource         = CLK_SRC_PLL0,
        .auxClockDivNumerator   = 1,
        .auxClockDivDenominator = 24
    },
    {
        .auxClockEnableMask     = AUX_CLK_MASK_SDIO,
        .auxClockSource         = CLK_SRC_PLL0,
        .auxClockDivNumerator   = 1,
        .auxClockDivDenominator = 2
    },
    {0, 0, 0, 0}, // Null Terminated List
};

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
// 6: Functions Implementation
// ----------------------------------------------------------------------------
void blocksResetSiliconSpecific(void) {
    DrvCprSysDeviceAction(SIPP_DOMAIN, ASSERT_RESET, DEV_SIPP_MIPI);
    DrvCprSysDeviceAction(UPA_DOMAIN, ASSERT_RESET,  APP_UPA_CLOCKS);
    DrvCprSysDeviceAction(MSS_DOMAIN, DEASSERT_RESET, -1);
    DrvCprSysDeviceAction(CSS_DOMAIN,  DEASSERT_RESET, -1);
    DrvCprSysDeviceAction(SIPP_DOMAIN, DEASSERT_RESET, -1);
    DrvCprSysDeviceAction(UPA_DOMAIN,  DEASSERT_RESET, -1);
}

int initClocks(uint32_t sys_clock_Hz) {
    s32 sc;

    if (sys_clock_Hz == 0) {
        mvLog(MVLOG_ERROR, "Don't set Sys Clock and set default 600M");
        sys_clock_Hz = 600; // 600MHz
    }
    swcLeonDataCacheFlush();
    // Configure the system
    sc = OsDrvCprInit();
    if (sc) {
        mvLog(MVLOG_ERROR, "LOS Drv Cpr Init Failed!");
        return sc;
    }

    sc = OsDrvCprOpen();
    if (sc) {
        mvLog(MVLOG_ERROR, "LOS Drv Cpr Open Failed!");
        return sc;
    }

    sc = OsDrvCprAuxClockArrayConfig(auxClk);
    if (sc) {
        mvLog(MVLOG_ERROR, "LOS Drv Cpr Aux Clock Array Config Failed!");
        return sc;
    }

    blocksResetSiliconSpecific();
    OsDrvCprSysDeviceAction(UPA_DOMAIN, DEASSERT_RESET, APP_UPA_CLOCKS);
    OsDrvCprSysDeviceAction(MSS_DOMAIN, DEASSERT_RESET, EXTRACLOCKS);
    sc = OsDrvTimerInit();
    if (sc) {
        mvLog(MVLOG_ERROR, "LOS Drv Timer Init Failed!");
        return sc;
    }

    // xeye V2 and xeye-face wouldn't those hardware module and disable clock
    sc = OsDrvCprSysDeviceAction(CSS_DOMAIN, DISABLE_CLKS, DEV_CSS_SPI1| DEV_CSS_SPI2 \
                                | DEV_CSS_I2S0 | DEV_CSS_I2S1 | DEV_CSS_I2S2 );
    if (sc) {
        mvLog(MVLOG_ERROR, "Disable useless Clock Failed!");
        return sc;
    }
    return sc;
}

extern int intOffset;
int initMemory(mv_mem_policy_t mem_policy) {
    s32 sc;

    DrvDdrInitialise(NULL);
    //sc = OsDrvCmxDmaInitDefault();
    sc = (int)OsDrvCmxDmaInit(13, intOffset, 1,1); 
    if (sc) {
        mvLog(MVLOG_ERROR, "LOS Drv CmxDma Default Init Failed!");
        return sc;
    }

    sc = OsDrvSvuInit();
    if (sc) {
        mvLog(MVLOG_ERROR, "LOS Drv Svu Init Failed!");
        return sc;
    }
    // Set the shave L2 Cache mode
    // sc = DrvShaveL2CacheSetMode(L2CACHE_CFG);
    sc = OsDrvShaveL2CacheInit(L2CACHE_CFG);
    if (sc) {
        mvLog(MVLOG_ERROR, "LOS Drv Shave L2 Cache Init Failed!");
        return sc;
    }
    if (mem_policy == DEFAULT_MEM_POLICY || mem_policy == XEYE_APP_MEM_POLICY) {
        int partitionNumber[SHAVES_USED];
        DrvLL2CFlushOpOnAllLines(LL2C_OPERATION_INVALIDATE, /*disable cache?:*/ 0);
        partitionNumber[0] = DrvShaveL2CacheSetupPartition(SHAVEPART128KB);
        partitionNumber[1] = DrvShaveL2CacheSetupPartition(SHAVEPART32KB);
        partitionNumber[2] = DrvShaveL2CacheSetupPartition(SHAVEPART32KB);
        partitionNumber[3] = DrvShaveL2CacheSetupPartition(SHAVEPART32KB);
        partitionNumber[4] = DrvShaveL2CacheSetupPartition(SHAVEPART32KB);

        for(int i = 0; i < SHAVES_USED; i++) {
            DrvShaveL2CacheSetInstrPartId(i, partitionNumber[1]);  //instruction part
        }
        for(int i = 6; i < SHAVES_USED; i++) {
            DrvShaveL2CacheSetLSUPartId(i, partitionNumber[0]);    //data part
            DrvShaveL2CacheSetWinPartId(i,SHAVEL2CACHEWIN_C,partitionNumber[2]); // win F used by SDL for common .data from DDR 0x1E...
        }
        for(int i = 0; i < 6; i++) {
            DrvShaveL2CacheSetLSUPartId(i, partitionNumber[3]);    //data part
            DrvShaveL2CacheSetWinPartId(i,SHAVEL2CACHEWIN_C,partitionNumber[4]); // win F used by SDL for common .data from DDR 0x1E...
        }
        sc = DrvShaveL2CacheAllocateSetPartitions();
        if(sc) {
            mvLog(MVLOG_ERROR, "LOS Drv Shave L2Cache Allocate Set Partitions Failed!");
            return sc;
        }
        DrvShaveL2CachePartitionFlushAndInvalidate(partitionNumber[0]);
        DrvShaveL2CachePartitionFlushAndInvalidate(partitionNumber[3]);
    } else if (mem_policy == HUATU_FACE_MEM_POLICY) {
        int part1Id, part2Id, part3Id, part4Id, part5Id, part6Id, part7Id, part8Id;
        int ret_code;
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART32KB, &part1Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART32KB, &part2Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART32KB, &part3Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART32KB, &part4Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART32KB, &part5Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART32KB, &part6Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART32KB, &part7Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART32KB, &part8Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        //Allocate Shave L2 cache set partitions
        ret_code = OsDrvShaveL2CacheAllocateSetPartitions();
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        //            // Invalidate SHAVE cache
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part1Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part2Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part3Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part4Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part5Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part6Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part7Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part8Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
    } else if (mem_policy == CROWD_MEM_POLICY) {
        int part1Id, part2Id, part3Id, part4Id;
        int ret_code;
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART64KB, &part1Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART64KB, &part2Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART64KB, &part3Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CGetPartition(SHAVEPART64KB, &part4Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        //Allocate Shave L2 cache set partitions
        ret_code = OsDrvShaveL2CacheAllocateSetPartitions();
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        //Invalidate SHAVE cache
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part1Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part2Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part3Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
        ret_code = OsDrvShaveL2CachePartitionInvalidate(part4Id);
        assert(ret_code == OS_MYR_DRV_SUCCESS);
    } else {
        mvLog(MVLOG_ERROR, "Not Support This Policy: %d Now", mem_policy);
    }
    
    return 0;
}

// We may upload new models and configuration files to SD card, enable sdio clock when needed
int XeyeClockControl(uint8_t usb_disable, uint8_t sdio_disable)
{
    uint32_t tmp0, tmp1, tmp2, tmp3 = 0;

    /*
       //AUX domain
        tmp0 = AUX_CLK_MASK_LCD | AUX_CLK_MASK_CIF1 \
          |AUX_CLK_MASK_MIPI_TX0 | AUX_CLK_MASK_MIPI_TX1\
          |AUX_CLK_MASK_I2S0|AUX_CLK_MASK_I2S1|AUX_CLK_MASK_I2S2;

        OsDrvCprSysDeviceAction(CSS_AUX_DOMAIN, DISABLE_CLKS, tmp0);
    */
    tmp1 =  DEV_CSS_SPI1| DEV_CSS_SPI2 \
           | DEV_CSS_I2S0 | DEV_CSS_I2S1 | DEV_CSS_I2S2 | DEV_CSS_USB;
    tmp0 = OsDrvCprSysDeviceAction(CSS_DOMAIN, DISABLE_CLKS, tmp1);

    // SIPP
    tmp2 = DEV_SIPP_MIPI_TX0 | DEV_SIPP_MIPI_TX1 | DEV_SIPP_MIPI_RX0 | DEV_SIPP_MIPI_RX1 | DEV_SIPP_MIPI_RX2 | DEV_SIPP_MIPI_RX3\
           | DEV_SIPP_MIPI |DEV_SIPP_EDGE_OP | DEV_SIPP_HARRIS ;
    tmp0 |=OsDrvCprSysDeviceAction(SIPP_DOMAIN, DISABLE_CLKS, tmp2);

    // MMS domain
    tmp3 = DEV_MSS_NAL | DEV_MSS_CIF1 | DEV_MSS_LCD ;
    tmp0 |= OsDrvCprSysDeviceAction(MSS_DOMAIN, DISABLE_CLKS, tmp3);

    // Specifically for USB
    if (usb_disable) {
        tmp1 = AUX_CLK_MASK_USB_PHY_EXTREFCLK | AUX_CLK_MASK_USB_CTRL_SUSPEND_CLK | AUX_CLK_MASK_USB_CTRL_REF_CLK | AUX_CLK_MASK_USB_PHY_REF_ALT_CLK;
        tmp0 |= OsDrvCprSysDeviceAction(CSS_AUX_DOMAIN, DISABLE_CLKS, tmp1);
        DrvCprPowerTurnOffIsland(POWER_ISLAND_USB);
    }

    if (sdio_disable) {
        tmp0 |= OsDrvCprSysDeviceAction(CSS_AUX_DOMAIN, DISABLE_CLKS, AUX_CLK_MASK_SDIO);
    } else {
        tmp0 |= OsDrvCprSysDeviceAction(CSS_AUX_DOMAIN, ENABLE_CLKS, AUX_CLK_MASK_SDIO);
    }
    return tmp0;
}

