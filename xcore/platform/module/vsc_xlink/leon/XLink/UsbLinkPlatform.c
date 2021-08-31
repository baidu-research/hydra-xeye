///
/// @file
/// @copyright All code copyright Movidius Ltd 2016, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Application configuration Leon header
///
#include "UsbLinkPlatform.h"

#include "stdio.h"
#include <assert.h>
#include <stdlib.h>
#include <rtems.h>
#include <rtems/libio.h>

#include <pthread.h>
#include <semaphore.h>

#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <registersMyriad.h>
#include <DrvRegUtilsDefines.h>
#include "mvMacros.h"

#ifndef USE_LINK_JTAG
#include "OsDrvUsbPhy.h"
#include "usbpumpdebug.h"
#include "usbpump_application_rtems_api.h"
#endif  /*USE_LINK_JTAG*/

#ifndef DISABLE_LEON_DCACHE
#define USBPUMP_MDK_CACHE_ENABLE        1
#else
#define USBPUMP_MDK_CACHE_ENABLE        0
#endif  /*DISABLE_LEON_DCACHE*/

//memory pool
#ifndef USB_LINK_MEM_POOL_SIZE
#define USB_LINK_MEM_POOL_SIZE (10*1024*1024)
#endif  /*USB_LINK_MEM_POOL_SIZE*/
#define USB_CHUNK_SIZE (5*1024*1024)

#define MEM_ALIGNMENT 64
#define MAX_SYMBOLS 50

uint32_t usedMemory;
static char* gl_devPathRead;
extern sem_t pingSem;

#ifdef USE_USB_VSC
#include "usb_vsc.h"
int vscFdWrite, vscFdRead;
#else
#ifdef USE_LINK_JTAG

    #ifndef USBLINK_PIPE_SECTION
    #define USBLINK_PIPE_SECTION ".ddr_direct.data"
    #endif  /*USBLINK_PIPE_SECTION*/

    #ifndef USBLINK_PIPE_SIZE
    #define USBLINK_PIPE_SIZE (100*1024)
    #endif  /*USBLINK_PIPE_SIZE*/

typedef struct {
    volatile u32  canaryStart;   // Used to detect corruption of queue
    volatile int  in;
    volatile int  out;
    volatile int  queueSize;
    volatile u32  canaryEnd;     // Used to detect corruption of queue
    volatile u8   buffer[USBLINK_PIPE_SIZE];
} tyMvConsoleQueue;
// pipe create USBLinkPipe -readsym mvUsbLinkTxQueue -writesym mvUsbLinkRxQueue -tcp 5678
// pipe USBLinkTx >>
// pipe USBLinkRx write @mvUsbLinkRxQueue
// pipe USBLinkRx >>
tyMvConsoleQueue mvUsbLinkTxQueue __attribute__((section(USBLINK_PIPE_SECTION)))=
{
    .canaryStart = 0x11223344,
    .in          = 0,
    .out         = 0,
    .queueSize   = USBLINK_PIPE_SIZE,
    .canaryEnd   = 0xAABBCCDD,
    .buffer      = {0},
};

tyMvConsoleQueue mvUsbLinkRxQueue __attribute__((section(USBLINK_PIPE_SECTION)))=
{
    .canaryStart = 0x11223344,
    .in          = 0,
    .out         = 0,
    .queueSize   = USBLINK_PIPE_SIZE,
    .canaryEnd   = 0xAABBCCDD,
    .buffer      = {0},
};

/*#################################################################################
###################################### INTERNAL ###################################
##################################################################################*/
static inline void * convertToUncachedAddr(void * addr)
{
    if ((u32)addr & 0x80000000)
        addr = (void*)((u32)addr | 0x40000000);
    else // Assume CMX
        addr = (void*)((u32)addr | 0x08000000);
    return addr;
}

// Blocking Queue Add
static int mvQueueAdd  (tyMvConsoleQueue * qPtr, char* buff, int num)
{
    tyMvConsoleQueue * q = convertToUncachedAddr(qPtr);
    int in = q->in;
    int out = q->out;
    int count;
    if (in == (( out - 1 + q->queueSize) % q->queueSize)){
        return 0; // pipe is full
    }else if (in >= out){
        count = q->queueSize - in;

    } else{ // in < out
        count = out - in - 1;
    }
    if (num < count){
        count = num;
    }
    memcpy((u8*)&q->buffer[in], buff, count);
    q->in = (q->in + count) % q->queueSize;
    return count;
}

// Blocking Queue Get
static int mvQueueGet  (tyMvConsoleQueue *qPtr, char* buff, int num)
{
    tyMvConsoleQueue * q = convertToUncachedAddr(qPtr);

    int in = q->in;
    int out = q->out;
    int count = 0;
    if(in == out){
        return -1;
    }else{
        if (in > out){
            count = in - out;
        } else{ // q->in < q->out
            count = q->queueSize - out;
        }
    }
    if (num < count){
        count = num;
    }
    memcpy(buff, (u8*)&q->buffer[out], count);
    q->out = (q->out +count) % q->queueSize;
    return count;
}

#else
#include "usb_uart.h"

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
    /* DebugMask */               UDMASK_ERRORS | UDMASK_ANY
    );
int usbFdWrite, usbFdRead;
#endif  /*USE_LINK_JTAG*/
#endif  /*USE_USB_VSC*/

/*#################################################################################
###################################### EXTERNAL ###################################
##################################################################################*/
int USBLinkWrite(void*fd, void* data, int num, unsigned int timeout)
{
    UNUSED(fd);
    int totalBytes = 0;
#ifndef USE_USB_VSC
    UNUSED(timeout);
#ifdef USE_LINK_JTAG
    while (totalBytes < num)
    {
        int ret = mvQueueAdd(&mvUsbLinkTxQueue, &((char*)data)[totalBytes], num - totalBytes);
        totalBytes += ret;
    }
#else
    while (totalBytes < num)
    {
        uint8_t acknowledge;
        int nread = 0;
        int nwrite = 0;
        int toWrite = (PACKET_LENGTH && (num - totalBytes > PACKET_LENGTH)) \
                        ? PACKET_LENGTH: num - totalBytes;

        while(toWrite > 0)
        {
            nwrite = write(usbFdRead, &((char*)data)[totalBytes], toWrite);
            if (nwrite >= 0)
            {
                totalBytes += nwrite;
                toWrite -= nwrite;
            }
            else
            {
                printf("Failed to write data from %x %d != %d\n",
                       (unsigned int)&((char*)data)[totalBytes], nwrite, num);
                assert(0);
            }
        }
//      printf("Wwrite %d %d\n", *(int*)data, totalBytes);
        while(nread < (int)sizeof(acknowledge))
        {
//          usleep(100);
            int count = read(usbFdRead, &acknowledge, sizeof(acknowledge));
            if (count > 0)
                nread += count;
        }

        if(nread != sizeof(acknowledge) || acknowledge != 0xEF)
        {
            printf("No acknowledge received %d %d\n", nread, acknowledge);
        }
//      printf("Wread %d %d\n", acknowledge, nread);
    }
#endif  /*USE_LINK_JTAG*/
#else
    while (totalBytes < num)
    {
        int towrite = num - totalBytes;
        if(towrite > USB_CHUNK_SIZE)
            towrite = USB_CHUNK_SIZE;

        int rc = write(vscFdWrite, &((char*)data)[totalBytes], towrite);
        UNUSED(timeout);
        if(rc < 0 && timeout)
            return rc;
        totalBytes += rc;
    }

#endif  /*USE_USB_VSC*/
    return num;
}

int USBLinkRead(void*fd, void* data, int num, unsigned int timeout)
{
    UNUSED(fd);
//     printf("Rread %d\n", num);
    int nread = 0;
#ifndef USE_USB_VSC
    UNUSED(timeout);
#ifdef USE_LINK_JTAG
    while (nread < num){
        int ret =  mvQueueGet(&mvUsbLinkRxQueue, &((char*)data)[nread], num - nread);
        if (ret != -1){
            nread += ret;
        }
        else
            usleep(1);
    }
#else
    while(nread < num)
    {
        int toRead = (PACKET_LENGTH && (num - nread > PACKET_LENGTH)) ? PACKET_LENGTH : num - nread;
        while(toRead > 0)
        {
            int count = read(usbFdWrite, &((char*)data)[nread], toRead);
            if (count > 0)
            {
//                printf("Rread %d\n", nread);
                nread += count;
                toRead -= count;
            }
        }
//        printf("Rread %d\n", nread);

        uint8_t ack = 0xEF;
        int nwrite = write(usbFdWrite, &ack, sizeof(ack));
        if(nwrite != sizeof(ack))
        {
            printf("Failed to write data %d != %d\n", nwrite, num);
        }
//        printf("Rwrite %d\n", ack);
    }
#endif  /*USE_LINK_JTAG*/
#else
    while (nread < num)
    {
        int toread = num - nread;
        if(toread > USB_CHUNK_SIZE)
            toread = USB_CHUNK_SIZE;
        int rc = read(vscFdRead, &((char*)data)[nread], toread);
        UNUSED(timeout);
        if(rc < 0 && timeout)
            return rc;
        nread += rc;
    }
#endif  /*USE_USB_VSC*/
    return nread;
}


int UsbLinkPlatformGetDeviceName(int index, char* name, int nameSize){
    UNUSED(index);
    UNUSED(name);
    UNUSED(nameSize);
    return -1;
}

int USBLinkPlatformResetRemote(void* fd)
{
    //TODO EMAN
    UNUSED(fd);
#ifndef USE_USB_VSC
#ifndef USE_LINK_JTAG
    close(usbFdWrite);
    if(usbFdRead != usbFdWrite)
        close(usbFdRead);
#endif  /*USE_LINK_JTAG*/
#else
    close(vscFdWrite);
    if(vscFdRead != vscFdWrite)
        close(vscFdRead);
#endif  /*USE_USB_VSC*/
    SET_REG_WORD(CPR_MAS_RESET_ADR, 0);
    return 0;
}

int UsbLinkPlatformConnect(const char* devPathRead,
                           const char* devPathWrite,
                           void** fd)
{
    strcpy(gl_devPathRead, devPathRead);
    UNUSED(devPathWrite);
    UNUSED(fd);
    return 0;
}

int UsbLinkPlatformInit(int loglevel)
{
    /*used in PC functions*/
    UNUSED(loglevel);

#ifndef USE_USB_VSC
#ifndef USE_LINK_JTAG
#ifndef MA2480
    osDrvUsbPhyParam_t initParam =
    {
        .enableOtgBlock    = USB_PHY_OTG_DISABLED,
        .useExternalClock  = USB_PHY_USE_EXT_CLK,
        .fSel              = USB_REFCLK_20MHZ,
        .refClkSel0        = USB_SUPER_SPEED_CLK_CONFIG,
        .forceHsOnly       = USB_PHY_HS_ONLY_OFF
    };
#else
    osDrvUsbPhyParam initParam =
    {
        .usb31_use_ext_ref_clk = USB_PHY_USE_EXT_CLK,
        .usb20_ref_clk_sel = USB_SUPER_SPEED_CLK_CONFIG,
        .usb31_ref_clk_sel = USB_SUPER_SPEED_CLK_CONFIG
    };
#endif /*MA2480*/

    OsDrvUsbPhyInit(&initParam);
    if (UsbPump_Rtems_DataPump_Startup(&sg_DataPump_AppConfig) != NULL)
    {
        printf("\n\nUsbPump_Rtems_DataPump_Startup()!\n\n\n");
    }
    else
    {
        printf("\n\nUsbPump_Rtems_DataPump_Startup() failed!\n\n\n");
        exit(1);
    }

    sleep(1);

    rtems_status_code sc;
    u32 vscMajor = 0;
    sc = rtems_io_register_driver(0, &UsbUartAdrTbl, &vscMajor);
    if (sc != RTEMS_SUCCESSFUL)
    {
        printf("USB UART Driver Register failed !!!!\n");
        rtems_fatal_error_occurred(sc);
    }

    // open USB
    usbFdWrite = open("/dev/usb0", O_RDWR);

    if(usbFdWrite < 0)
    {
        printf("No USB device !!!!\n");
    }

    // open USB
    usbFdRead = (gl_devPathRead && *gl_devPathRead == '=') ?
                    usbFdWrite : open("/dev/usb1", O_RDWR);

    if(usbFdRead < 0)
    {
        printf("No USB device !!!!\n");
    }
#endif  /*USE_LINK_JTAG*/
#else
    if (OsDrvUsbPhyInit(NULL)){
        printf("\n USB PHY module initialization failed for USB VSC!\n\n\n");
    };

    rtems_status_code sc;
    u32 vscMajor = 0;
    sc = rtems_io_register_driver(0, &VSCCompAdrTbl, &vscMajor);
    if (sc != RTEMS_SUCCESSFUL)
    {
        printf("USB VSC Driver Register failed !!!!\n");
        rtems_fatal_error_occurred(sc);
    }
    vscFdWrite = vscFdRead = open(USB_VSC_DEVNAME, O_RDWR);

    if(vscFdWrite < 0)
    {
        perror("No VSC_USB device !!!!\n");
    }
#endif  /*USE_USB_VSC*/

    // printf("waiting\n");
    // sem_wait(&pingSem);
    // printf("********************************************************\n");
    return 0;
}

void deallocateData(void* ptr,uint32_t size, uint32_t alignment)
{
    UNUSED(alignment);
    if (!ptr)
        return;
    free(ptr);
    usedMemory -= size;
}

void* allocateData(uint32_t size, uint32_t alignment)
{
    assert(MEM_ALIGNMENT % alignment == 0);
    void* ret = NULL;
    if (usedMemory + size > USB_LINK_MEM_POOL_SIZE) {
        printf("Not enough memory %ld %ld %d\n",
            usedMemory, size, USB_LINK_MEM_POOL_SIZE);
        return ret;
    }
    posix_memalign(&ret, alignment, size);
    usedMemory += size;
    return ret;
}

int UsbLinkPlatformBootRemote(const char* deviceName, const char* binaryPath)
{
    UNUSED(deviceName);
    UNUSED(binaryPath);
    return -1; // would be nice to boot the PC, but let's not do that.
}

int USBLinkIsReconnect(void) {
    return is_usb_reconnected();
}

int USBLinkResetReconnectFlag(void) {
    return reset_usb_reconnect_flag();
}

/* end of file */
