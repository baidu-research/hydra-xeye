/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/
 
#include <stdlib.h>
#include <stdio.h>
#include <rtems.h>
#include <rtems/bspIo.h>
#include <bsp.h>
#include <sys/time.h>

#include "OsDrvUsbPhy.h"
#include "OsDrvCpr.h"
#include "usbpumpdebug.h"
#include "usbpump_application_rtems_api.h"
#include "vsc2app_outcall.h"
#include "usbpump_vsc2app.h"
#include "xeye_usb_api.h"
#define MVLOG_UNIT_NAME xeye_usb_api
#include "mvLog.h"

#ifndef DISABLE_LEON_DCACHE
# define USBPUMP_MDK_CACHE_ENABLE   1
#else
# define USBPUMP_MDK_CACHE_ENABLE   0
#endif

#ifndef TIMEOUT
# define TIMEOUT 0
#endif

#ifndef WAIT_TIME
# define WAIT_TIME 200
#endif

#define FIRST_EP_NO 0
#define LAST_EP_NO 2

// USB VSC Handler
extern USBPUMP_VSC2APP_CONTEXT*   pSelf;

// 4: Static Local Data
// ----------------------------------------------------------------------------
static USBPUMP_APPLICATION_RTEMS_CONFIGURATION sg_DataPump_AppConfig =
    USBPUMP_APPLICATION_RTEMS_CONFIGURATION_INIT_V3(
        /* nEventQueue */   64,
        /* pMemoryPool */   NULL,
        /* nMemoryPool */   0,
        /* DataPumpTaskPriority */  100,
        /* DebugTaskPriority */   200,
        /* UsbInterruptPriority */  10,
        /* pDeviceSerialNumber */ NULL,
        /* pUseBusPowerFn */    NULL,
        /* fCacheEnabled */   USBPUMP_MDK_CACHE_ENABLE,
        /* DebugMask */     UDMASK_ANY | UDMASK_ERRORS,
        /* pPlatformIoctlFn */    NULL,
        /* fDoNotWaitDebugFlush */  0
    );

static int sg_usb_connected;  // 0 -> not connected, 1 -> connected

// Required for synchronisation between internal USB thread and our threads
static int createSemaphores() {
    int i, status;

    for (i = 0; i < USBPUMP_VSC2APP_NUM_EP_IN; i++) {
        status = rtems_semaphore_create(rtems_build_name('I', 'N', '_', '0' + i), 0,
                                        RTEMS_SIMPLE_BINARY_SEMAPHORE, 0, &pSelf->semWriteId[i]);

        if (status != RTEMS_SUCCESSFUL) {
            return 1;
        }
    }

    for (i = 0; i < USBPUMP_VSC2APP_NUM_EP_OUT; i++) {
        status = rtems_semaphore_create(rtems_build_name('O', 'U', 'T', '0' + i), 0,
                                        RTEMS_SIMPLE_BINARY_SEMAPHORE, 0, &pSelf->semReadId[i]);

        if (status != RTEMS_SUCCESSFUL) {
            return 1;
        }
    }

    return 0;
}
static int destroySemaphores(void) {
    int i, status;

    for (i = 0; i < USBPUMP_VSC2APP_NUM_EP_IN; i++) {
        status = rtems_semaphore_delete(pSelf->semReadId[i]);

        if (status != RTEMS_SUCCESSFUL) {
            mvLog(MVLOG_ERROR, "Endpoint in: %d, rtems_semaphore_delete failed: %s", i, rtems_status_text(status));
            return status;
        }
    }

    for (i = 0; i < USBPUMP_VSC2APP_NUM_EP_OUT; i++) {
        status = rtems_semaphore_delete(pSelf->semWriteId[i]);

        if (status != RTEMS_SUCCESSFUL) {
            mvLog(MVLOG_ERROR, "Endpoint out:%d, rtems_semaphore_delete failed: %s", i, rtems_status_text(status));
            return status;
        }
    }

    return RTEMS_SUCCESSFUL;
}

int XeyeUsbVscWrite(char* buf, int size, int timeout) {
    //UNUSED(args);
    int status = VSC_ERROR;
    int ep_no = 0;
    int i = 0;

    if (sg_usb_connected) {
        UsbVscAppWrite(pSelf, size, buf, ep_no);
        status = rtems_semaphore_obtain(pSelf->semWriteId[ep_no], RTEMS_WAIT, timeout);

        switch (status) {
        case RTEMS_TIMEOUT:
            mvLog(MVLOG_ERROR, "Write semaphore obtain timeout on EP:%d!", ep_no);
            status = VSC_TIMEOUT;
            break;

        case RTEMS_SUCCESSFUL:
            status = VSC_SUCCESS;
            break;

        default:
            mvLog(MVLOG_ERROR, "Error %d on EP:%d", status, ep_no);
            status = VSC_ERROR;
            break;
        }
    } else {
        // just sleep for a while
        if (timeout != 0) {
            rtems_task_wake_after(timeout);
        } else {
            rtems_task_wake_after(1000);
        }

        status = VSC_NOT_CONNECTED;
    }

    return status;
}

int XeyeUsbVscRead(char* buf, int size, int timeout) {
    //UNUSED(args);
    int status = VSC_ERROR;
    int ep_no = 0;

    if (sg_usb_connected) {
        UsbVscAppRead(pSelf, size, buf, ep_no);
        status = rtems_semaphore_obtain(pSelf->semReadId[ep_no], RTEMS_WAIT, timeout);

        switch (status) {
        case RTEMS_TIMEOUT:
            // mvLog(MVLOG_ERROR, "Read semaphore obtain timeout on EP:%d!", ep_no);
            status = VSC_TIMEOUT;
            break;

        case RTEMS_SUCCESSFUL:
            status = VSC_SUCCESS;
            break;

        default:
            mvLog(MVLOG_ERROR, "Error %d on EP:%d", status, ep_no);
            status = VSC_ERROR;
            break;
        }
    } else {
        // just sleep for a while
        if (timeout != 0) {
            rtems_task_wake_after(timeout);
        } else {
            rtems_task_wake_after(1000);
        }

        status = VSC_NOT_CONNECTED;
    }

    return status;
}

// Delete all semaphores when detect usb interface down
void usb_interface_down_callback(int arg) {
    // [NOTE]: Must assign sg_usb_connected to 0 firstly
    sg_usb_connected = 0;
    destroySemaphores();
    mvLog(MVLOG_INFO, ">>>>>>>>> USB disconnected!");
}

// Create new semaphores when detect usb interface up
void usb_interface_up_callback(int arg) {
    if (sg_usb_connected == 0) {
        if (createSemaphores() != 0) {
            mvLog(MVLOG_ERROR, "Error creating semaphores!");
        }

        sg_usb_connected = 1;
    }

    mvLog(MVLOG_INFO, ">>>>>>>>> USB connected!");
}

// Get usb interface status
int XeyeUsbVscConnectStatus(void) {
    return sg_usb_connected;
}

int XeyeDrvUsbInit(void) {
    int status = VSC_SUCCESS;
    status = OsDrvUsbPhyInit(NULL);

    if (status != RTEMS_SUCCESSFUL) {
        return VSC_ERROR;
    }

    if (!UsbPump_Rtems_DataPump_Startup(&sg_DataPump_AppConfig)) {
        mvLog(MVLOG_ERROR, "\n\nRtems_DataPump_Startup() failed!\n\n\n");
        return VSC_ERROR;
    }

    sg_usb_connected = 0;
    pSelf->usbInterfaceUpCB = usb_interface_up_callback;
    pSelf->usbInterfaceDownCB = usb_interface_down_callback;

    // Needed so that the datapump has time to do interface up
    rtems_task_wake_after(1000);

    return status;
}

