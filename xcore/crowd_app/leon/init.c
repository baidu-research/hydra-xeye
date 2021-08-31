///
/// @file
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Variation of "IpipePPTest09, IpipePPTest09_a
///            PP inputs 960x128 and outputs 480x64 RGB data for VideoSipp
///            LOS starts LRT which executes the PP test.
///

// 1: Includes
// ----------------------------------------------------------------------------
#include <DrvLeon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <DrvCpr.h>
#include <errno.h>
#include "OsDrvUsbPhy.h"
#include "cam_config.h"
#include "LeonIPCApi.h"
#include "usbpumpdebug.h"
#include "usbpump_application_rtems_api.h"
#include <rtems/fsmount.h>
#include <rtems/bdpart.h>
#include <OsDrvSdio.h>
#include "DrvCDCEL.h"
#include "rtems_config.h"
#include <DrvGpio.h>
#include "DrvTimer.h"
#include "model.h"
#include <fcntl.h>
#include <assert.h>
#include <rtems.h>
#include <rtems/bspIo.h>
#include <mv_types.h>
#include "mvHelpersApi.h"
#include <OsBrdMv0182.h>
#include "xeye_uart.h"
#include "model.h"
#include "DrvTimer.h"
#include <OsDrvShaveL2Cache.h>
#include "xeye_sdcard.h"
// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------
#ifndef DISABLE_LEON_DCACHE
# define USBPUMP_MDK_CACHE_ENABLE        1
#else
# define USBPUMP_MDK_CACHE_ENABLE        0
#endif
#define L2CACHE_CFG                (SHAVE_L2CACHE_NORMAL_MODE)
#define SDIO_DEVNAME_USED                           "/dev/sdc0"

#define DDR_BSS  __attribute__((section(".ddr.bss")))


// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
//Entry point of the leon_rt processor
extern u32* lrt_start;
// IPC channel used to send USB speed status from LOS to LRT
leonIPCChannel_t __attribute__((section(".cmx_direct.data"))) lrt_LOStoLRTChannel;
uint32_t __attribute__((section(".cmx_direct.data"))) messagePool[MSG_QUEUE_SIZE* MSG_SIZE];

// 4: Static Local Data
// ----------------------------------------------------------------------------
static USBPUMP_APPLICATION_RTEMS_CONFIGURATION sg_DataPump_AppConfig =
    USBPUMP_APPLICATION_RTEMS_CONFIGURATION_INIT_V1(
        /* nEventQueue */             64,
        /* pMemoryPool */             NULL,
        /* nMemoryPool */             0,
        /* DataPumpTaskPriority */    100,
        /* DebugTaskPriority */       200,
        /* UsbInterruptPriority */    10,
        /* pDeviceSerialNumber */     NULL,
        /* pUseBusPowerFn */          NULL,
        /* fCacheEnabled */           USBPUMP_MDK_CACHE_ENABLE,
        /* DebugMask */               UDMASK_ANY | UDMASK_ERRORS
    );

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------

// 6: Functions Implementation
// ----------------------------------------------------------------------------
void* os_Init(void* args) {
    UNUSED(args);
    s32 sc;
    osDrvUsbPhyParam_t initParam = {
        .enableOtgBlock    = USB_PHY_OTG_DISABLED,
        .useExternalClock  = USB_PHY_USE_EXT_CLK,
        .fSel              = USB_REFCLK_20MHZ,
        .refClkSel0        = USB_SUPER_SPEED_CLK_CONFIG,
        .forceHsOnly       = USB_PHY_HS_ONLY_OFF
    };

    sc = LeonIPCTxInit(&lrt_LOStoLRTChannel, messagePool, MSG_QUEUE_SIZE, MSG_SIZE);

    if (sc) {
        exit(sc);
    }
    DrvLeonRTStartup((u32)&lrt_start);
    DrvLeonRTWaitForBoot();
    printf("LeonOS drive LeonRT init finish\n");
    XeyeDrvSdcardInit();
    OsDrvUsbPhyInit(&initParam);
    if (UsbPump_Rtems_DataPump_Startup(&sg_DataPump_AppConfig) != NULL) {
        printf("\n\nUsbPump_Rtems_DataPump_Startup()!\n\n\n");
    } else {
        printf("\n\nUsbPump_Rtems_DataPump_Startup() failed!\n\n\n");
        exit(1);
    }
}
