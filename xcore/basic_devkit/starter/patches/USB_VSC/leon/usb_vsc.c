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
#ifdef MA2480
#include <fcntl.h>
#include <sys/ioctl.h>
#include <OsDrvEfuse.h>
#else
#include <DrvEfuse.h>
#endif
#include <unistd.h>
#include <rtems/libio.h>

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define ENDPOINT1 0
#ifdef MA2480
#define SERIAL_START_BIT 256
#define SERIAL_END_BIT 323
#define SERIAL_SIZE ((SERIAL_END_BIT - SERIAL_START_BIT + 7)/8) //round up
#define SERIAL_SIZE_32 ((SERIAL_SIZE+3)/4) //round up
#else
#define SERIAL_START_BIT 544
#define SERIAL_END_BIT 596
#define SERIAL_SIZE 7
#endif

extern USBPUMP_VSC2APP_CONTEXT *    pSelf;

#ifdef USB_STANDALONE_MEM_POOL
#ifndef USB_POOL_SZ
#define USB_POOL_SZ	10*1024*1024
#endif
uint8_t USB_Mem_Pool[USB_POOL_SZ] __attribute__((aligned(32))) __attribute__((section(".ddr.bss")));

static USBPUMP_APPLICATION_RTEMS_CONFIGURATION sg_DataPump_AppConfig =
USBPUMP_APPLICATION_RTEMS_CONFIGURATION_INIT_V3(
    /* nEventQueue */       64,
    /* pMemoryPool */       USB_Mem_Pool,
    /* nMemoryPool */       USB_POOL_SZ,
    /* DataPumpTaskPriority */  100,
    /* DebugTaskPriority */     200,
    /* UsbInterruptPriority */  10,
    /* pDeviceSerialNumber */   NULL,
    /* pUseBusPowerFn */        NULL,
    /* fCacheEnabled */     USBPUMP_MDK_CACHE_ENABLE,
    /* DebugMask */         UDMASK_ANY | UDMASK_ERRORS,
    /* pPlatformIoctlFn */    NULL,
    /* fDoNotWaitDebugFlush */  0
    );
#else
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
#endif

// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
extern USBPUMP_VSC2APP_CONTEXT *    pSelf;

// 4: Static Local Data
// ----------------------------------------------------------------------------
static int readtimeout, writetimeout;

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------

// 6: Functions Implementation
// ----------------------------------------------------------------------------
#ifdef MA2480
rtems_status_code RegisterAndInitializeEFuseDriver(bool gp_state_machine_en)
{
    rtems_status_code sc = RTEMS_SUCCESSFUL;
    uint32_t major;

    // Register the EFuse driver for the block under test.
    sc = rtems_io_register_driver(0, &efuse_drv_tbl, &major);

    if (RTEMS_SUCCESSFUL == sc) {
        printf("EFuses driver registered\n\n");
    } else {
        printf("EFuses driver failed to register\n\n");
        return -1;
    }

    // Initialize the efuse driver with the HW state machine disabled.
    OsDrvEfuseHWConfig efuse_hw_cfg;

    efuse_hw_cfg.gp_state_machine_en = gp_state_machine_en;
    efuse_hw_cfg.banks_en = OS_DRV_EFUSE_GP_DUAL_BANK_EN;

    // Initialize the efuse driver.
    sc = rtems_io_initialize(major, 0, &efuse_hw_cfg);

    if (RTEMS_SUCCESSFUL == sc) {
        printf("%s(): Initialization succeed.", __func__);
    } else {
        printf("%s(): Initialization failed.", __func__);
    }

    return sc;
}

rtems_status_code OpenEfuseFD(int* fd)
{
    // Open the interface.
    *fd = open(OS_DRV_EFUSE_DEVNAME, O_RDONLY);

    if(*fd < 0) {
        perror("rtems_efuse_open failed");
        return -1;
    }
    return 0;
}
#endif

int getDFuseNumber(char serial_str[], int str_size)
{
#ifdef MA2480
    uint32_t read_back[SERIAL_SIZE_32] = {0};
    int fd;
    rtems_status_code sc = RegisterAndInitializeEFuseDriver(FALSE);
    if (sc != RTEMS_SUCCESSFUL) {
        return -1;
    }
    if (OpenEfuseFD(&fd)) {
        return -1;
    }
    OsDrvEfuseConfig efuse_config = {
        .fuse_id = 0,
        .fuse_mask = 0,
    };

    int i;
    for (i = 0; i < SERIAL_SIZE_32; i++) {
        // Read efuse
        efuse_config.fuse_id = SERIAL_START_BIT + i*32;
        efuse_config.fuse_read_back = &read_back[i];

        sc = ioctl(fd, IOCTL_READ_32_GP_FUSES, &efuse_config);
        if (RTEMS_SUCCESSFUL != sc) {
            printf("%s() : Error reading efuse \n", __func__);
            close(fd);
            return -2;
        }

        //printf("efuse %lu read with value %lu",
        //    efuse_config.fuse_id,
        //    read_back[i]);
    }

    //copy to serial_str as hex numbers
    char* dnumber = (char*) read_back;
    dnumber[SERIAL_SIZE-1] = dnumber[SERIAL_SIZE-1] & 0xF0; //last 4 bits are garbage
    int temp;
    for (i = 0; i < SERIAL_SIZE; i++) {
        if ((temp = snprintf(serial_str + 2*i, str_size - 2*i, "%02X", dnumber[i])) != 2) {
            printf("Error generating serial string %d\n", temp);
            close(fd);
            return -3;
        }
    }

    // Close everything opened.
    sc = close(fd);

    if(RTEMS_SUCCESSFUL != sc) {
        printf("%s() : rtems_efuse_close failed", __func__);
        return -4;
    }
    return 0;

#else
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
#endif
}

static inline rtems_status_code usb_vsc_init(rtems_device_major_number major,
                            rtems_device_minor_number minor)
{
    rtems_status_code rc;
#ifdef MA2480
    char serial_str[] = "deadbeefdeadbeefd";
#else
    char serial_str[] = "deadbeefdeadbe";
#endif
    OsDrvUsbPhyInit(NULL);
    if (getDFuseNumber(serial_str, sizeof(serial_str)) == 0) {
        sg_DataPump_AppConfig.pDeviceSerialNumber = serial_str;
    }

    if (!UsbPump_Rtems_DataPump_Startup(&sg_DataPump_AppConfig))
    {
        printf("\n\nUPF_DataPump_Startup() failed!\n\n\n");
    }

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

/*#################################################################################
################################### libio functions ###############################
##################################################################################*/

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
