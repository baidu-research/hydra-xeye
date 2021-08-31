///
/// @file usb_vsc.c
/// @copyright All code copyright Movidius Ltd 2017, all rights reserved
///            For License Warranty see: common/license.txt
/// @ingroup USB_VSC
/// @{
/// @brief     USB VSC API Functions.
///

// 1: Includes
// ----------------------------------------------------------------------------
#include <stdio.h>
#include "usb_vsc.h"

#include "OsDrvUsbPhy.h"

#include "usbpumpdebug.h"
#include "usbpump_application_rtems_api.h"

#include "vsc2app_outcall.h"
#include "usbpump_vsc2app.h"
#include <DrvEfuse.h>

#include <unistd.h>
#include <rtems/libio.h>
#define MVLOG_UNIT_NAME usb_vsc
#include <mvLog.h>

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define ENDPOINT1 0
#define SERIAL_START_BIT 544
#define SERIAL_END_BIT 596
#define SERIAL_SIZE 7

extern USBPUMP_VSC2APP_CONTEXT *    pSelf;

static USBPUMP_APPLICATION_RTEMS_CONFIGURATION sg_DataPump_AppConfig =
USBPUMP_APPLICATION_RTEMS_CONFIGURATION_INIT_V5(
    /* nEventQueue */       64,
    /* pMemoryPool */       NULL,
    /* nMemoryPool */       0,
    /* DataPumpTaskPriority */  100,
    /* DebugTaskPriority */     200,
    /* UsbInterruptPriority */  10,
    /* pDeviceSerialNumber */   NULL,
    /* pUseBusPowerFn */        NULL,
    /* fCacheEnabled */     USBPUMP_MDK_CACHE_ENABLE,
    /* DebugMask */         UDMASK_ANY | UDMASK_ERRORS,
    /* pPlatformIoctlFn */    NULL,
    /* fDoNotWaitDebugFlush */  0,
    /* pDeviceMacAddress */     NULL,
    /* OsStringDescriptorVendorCode */ 0x04  /* non-zero value to enable OS descriptor */
    );

// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
extern USBPUMP_VSC2APP_CONTEXT *    pSelf;

// 4: Static Local Data
// ----------------------------------------------------------------------------
static int readtimeout, writetimeout;

static int sg_usb_connected;  // 0 -> not connected, 1 -> connected
static int sg_usb_reconnect;  // 0 -> no reconnect, 1 -> reconnect
// 5: Static Function Prototypes
// ----------------------------------------------------------------------------

// 6: Functions Implementation
// ----------------------------------------------------------------------------
int getDFuseNumber(char serial_str[], int str_size)
{
    u8 dnumber[SERIAL_SIZE];
    efuseSet set;
    int res;
    int i;

    if (str_size/2 > SERIAL_SIZE)
        return -1; //shouldnt happen

     // Read the entire set of EFuses.
    if((res = DrvEfuseReadEFuses(&set, APPLY_FIX, NORMAL_MODE)) != 0) {
        printf("DrvEfuseReadEFuses - error %d\n", res);
    } else {
        printf("EFuses read successfully\n\n");
        DrvEfuseGetValuePtr(&set, SERIAL_START_BIT, SERIAL_END_BIT, dnumber);

        //reset last 3 bits since they might be garbage
        dnumber[SERIAL_SIZE-1] = dnumber[SERIAL_SIZE-1] & 0xF8;
        int temp;
        for (i = 0; i < SERIAL_SIZE; i++) {
            if ((temp = snprintf(serial_str + 2*i, str_size - 2*i, "%02X", dnumber[i])) != 2)
            {
                printf("Error generating serial string %d\n", temp);
                return -1;
            }
        }
    }

    return res;
}

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

// Delete all semaphores when detect usb interface down
void usb_interface_down_cb_xlink(int arg) {
    // [NOTE]: Must assign sg_usb_connected to 0 firstly
    sg_usb_connected = 0;
    // destroySemaphores();
    mvLog(MVLOG_INFO, ">>>>>>>>> USB disconnected!");
}

// Create new semaphores when detect usb interface up
void usb_interface_up_cb_xlink(int arg) {
    if (sg_usb_connected == 0) {
        // if (createSemaphores() != 0) {
        //     mvLog(MVLOG_ERROR, "Error creating semaphores!");
        // }

        sg_usb_connected = 1;
        sg_usb_reconnect = 1;
    }

    mvLog(MVLOG_INFO, ">>>>>>>>> USB connected!");
}

int is_usb_reconnected(void) {
    return sg_usb_reconnect;
}

int reset_usb_reconnect_flag(void) {
    sg_usb_reconnect = 0;
    return 0;
}

static inline rtems_status_code usb_vsc_init(rtems_device_major_number major,
                            rtems_device_minor_number minor)
{
    rtems_status_code rc;
    char serial_str[] = "deadbeefdeadbe";

    OsDrvUsbPhyInit(NULL);
    if (getDFuseNumber(serial_str, sizeof(serial_str)) == 0) {
        sg_DataPump_AppConfig.pDeviceSerialNumber = serial_str;
    }

    if (!UsbPump_Rtems_DataPump_Startup(&sg_DataPump_AppConfig))
    {
        printf("\n\nUPF_DataPump_Startup() failed!\n\n\n");
    }
    // [NOTE]: The semaphores must be created after data pump startup, or driver register will be failed
    if (createSemaphores() != 0) {
        mvLog(MVLOG_ERROR, "Error creating semaphores!");
    }

    // TODO(hyx): add callback functions in the future
    sg_usb_connected = 1;
    sg_usb_reconnect = 0;
    pSelf->usbInterfaceUpCB = usb_interface_up_cb_xlink;
    pSelf->usbInterfaceDownCB = usb_interface_down_cb_xlink;
    //TODO:This sleep is required as sometimes the semaphore is not created before
    //the obtain below is called. Need to investigate how to solve without sleep
    usleep(10000);

    //first read semaphore will be used to wait the interface up event
    rc = rtems_semaphore_obtain(pSelf->semReadId[0], RTEMS_WAIT, 0);
    if (rc == RTEMS_SUCCESSFUL){
        rc = rtems_io_register_name(USB_VSC_DEVNAME,
                                    major,
                                    minor);
    }
    readtimeout = writetimeout = 0;

    return rc;
}

static inline rtems_status_code usb_vsc_read(void *buffer,
                            size_t size,
                            int timeout,
                            int endpoint){
    rtems_status_code rc;
    if(!readtimeout){
        UsbVscAppRead(pSelf, size, buffer, endpoint);
    }
    readtimeout = 0;
    rc = rtems_semaphore_obtain(pSelf->semReadId[endpoint],
                                RTEMS_WAIT,
                                timeout);
    if(rc == RTEMS_TIMEOUT){
        readtimeout = 1;
    }
    return rc;
}

static inline rtems_status_code usb_vsc_write(void *buffer,
                            size_t size,
                            int timeout,
                            int endpoint){
    rtems_status_code rc;
    if(!writetimeout){
        UsbVscAppWrite(pSelf, size, buffer, endpoint);
    }
    writetimeout = 0;
    rc = rtems_semaphore_obtain(pSelf->semWriteId[endpoint],
                                RTEMS_WAIT,
                                timeout);
    if (rc == RTEMS_TIMEOUT){
        writetimeout = 1;
    }
    return rc;
}

static rtems_status_code VSCInitialize(rtems_device_major_number major,
                            rtems_device_minor_number minor,
                            void *args)
{
    UNUSED(major);
    UNUSED(minor);
    UNUSED(args);

    rtems_status_code sc;
    sc = usb_vsc_init(major, minor);
    return sc;
}

static rtems_status_code VSCOpen(rtems_device_major_number major,
                            rtems_device_minor_number minor,
                            void *args)
{
    UNUSED(major);
    UNUSED(minor);
    UNUSED(args);
    // There is nothing to do here. Return success
    return RTEMS_SUCCESSFUL;
}

static rtems_status_code VSCClose(rtems_device_major_number major,
                            rtems_device_minor_number minor,
                            void *args)
{
    UNUSED(major);
    UNUSED(minor);
    UNUSED(args);

    return RTEMS_NOT_IMPLEMENTED;
}

static rtems_status_code VSCRead(rtems_device_major_number major,
                            rtems_device_minor_number minor,
                            void *args)
{
    UNUSED(major);
    UNUSED(minor);
    UNUSED(args);

    rtems_status_code sc;
    rtems_libio_rw_args_t *rwargs = args;

    if(rwargs == NULL || rwargs->buffer == NULL)
        return RTEMS_INVALID_ADDRESS;

    sc = usb_vsc_read(rwargs->buffer,                         //buffer 
                    rwargs->count,                            //size
                    1 * rtems_clock_get_ticks_per_second(),   //timeout (1sec)           
                    ENDPOINT1);                               //endpoint    
   
    if (sc == RTEMS_SUCCESSFUL){
        rwargs->bytes_moved = rwargs->count;
    }else{
        rwargs->bytes_moved = 0;
    }
    return sc;
}

static rtems_status_code VSCWrite(rtems_device_major_number major,
                            rtems_device_minor_number minor,
                            void *args)
{
    UNUSED(major);
    UNUSED(minor);
    UNUSED(args);

    rtems_status_code sc;
    rtems_libio_rw_args_t *rwargs = args;

    if(rwargs == NULL || rwargs->buffer == NULL)
        return RTEMS_INVALID_ADDRESS;

    sc = usb_vsc_write(rwargs->buffer,                        //buffer 
                    rwargs->count,                            //size
                    1 * rtems_clock_get_ticks_per_second(),   //timeout (1sec)           
                    ENDPOINT1);                               //endpoint                    

    if (sc == RTEMS_SUCCESSFUL){
        rwargs->bytes_moved = rwargs->count;
    }else{
        rwargs->bytes_moved = 0;
    }
    return sc;
}

static rtems_status_code VSCControl(rtems_device_major_number major,
                            rtems_device_minor_number minor,
                            void *args)
{
    UNUSED(major);
    UNUSED(minor);
    UNUSED(args);

    rtems_status_code sc = RTEMS_SUCCESSFUL;
    rtems_libio_ioctl_args_t *ctl = args;

    if (ctl == NULL || ctl->buffer == NULL)
        return RTEMS_INVALID_ADDRESS;

    switch (ctl->command)
    {
        default:
            sc = RTEMS_NOT_DEFINED;
    }
    return sc;
}

rtems_driver_address_table VSCCompAdrTbl = {
    VSCInitialize,
    VSCOpen,
    VSCClose,
    VSCRead,
    VSCWrite,
    VSCControl
};

///@}
