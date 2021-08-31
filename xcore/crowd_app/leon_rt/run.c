///
//
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief
///

// 1: Includes
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rtems.h>
#include <swcShaveLoader.h>
#include <swcTestUtils.h>
#include <DrvShaveL2Cache.h>
#include "DrvCDCEL.h"
#include "DrvGpio.h"
#include "Cam214SIPPDebayerHdmi.h"
#include "sippRGB.h"
#include "Fp16Convert.h"
#include "debug.h"
#include "mvHelpersApi.h"
#include "xeye_eeprom.h"

#include "DrvCDCEL.h"
#include "DrvCpr.h"
#include "CamGenericApi.h"
#include "sipp.h"
#include "sippTestCommon.h"
#include "DrvLeon.h"
#include "swcFrameTypes.h"
#include "cam_config.h"
#include "LeonIPCApi.h"
#include "DrvTimer.h"
#include "define.h"
#include "pubdef.h"
#include "DrvUart.h"
#include <DrvRegUtils.h> //register access
#include "xeye_aec_algo.h"
#include "xeye_firmware_version.h"
#include "platform_sensor_api.h"
#include "lrt_plat_common.h"
#include <mvLog.h>


// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define FRAME_READY 1
// 3: Global Data (Only if absolutely necessary)

leonIPCChannel_t __attribute__((section(".cmx_direct.data"))) LRTtoLOSChannel;
extern leonIPCChannel_t LOStoLRTChannel;
uint32_t __attribute__((section(".cmx_direct.data"))) messagePool[MSG_QUEUE_SIZE* MSG_SIZE];
static uint32_t receivedMessage;
static uint32_t currentSpeed;
static uint32_t sensor_mipi_channel = 0;
extern RunningMode g_running_mode;
volatile ALIGNED(4)  __attribute__((section(" .cmx_direct.data")))  u32 usbFrameCtr = FIRST_OUTGOING_BUF_ID;

uint8_t* g_pucConvBufY __attribute__((section(" .cmx_direct.bss")));
uint8_t* g_pucConvBufU;
uint8_t* g_pucConvBufV;

#define LEON_HEAP_SIZE 30000000
u8 leonHeap[LEON_HEAP_SIZE] DDR_AREA;

volatile ALIGNED(4)  __attribute__((section(".ddr_direct.bss"))) int modelInitDone ;
volatile ALIGNED(4)  __attribute__((section(".ddr_direct.bss"))) int g_ccm_index = 0;

// to correctly send frames on USB from DDR, each buffer must be aligned to 128 bytes
// each frame will be send in an USB bulk transfer and it must be preceded by the USB stream header
u8 DDR_AREA usbBuf1[PAYLOAD_HEADER_OFFSET + RES_WIDTH_MAX * RES_HEIGHT_MAX * 3 ];
u8 DDR_AREA usbBuf2[PAYLOAD_HEADER_OFFSET + RES_WIDTH_MAX * RES_HEIGHT_MAX * 3 ];
u8 DDR_AREA usbBuf3[PAYLOAD_HEADER_OFFSET + RES_WIDTH_MAX * RES_HEIGHT_MAX * 3 ];
u8 DDR_AREA usbBuf4[PAYLOAD_HEADER_OFFSET + RES_WIDTH_MAX * RES_HEIGHT_MAX * 3 ];
u8 DDR_AREA rgbBuf[RES_WIDTH_MAX * RES_HEIGHT_MAX * 3];
u8 DDR_AREA lumaBuf[RES_WIDTH_MAX * RES_HEIGHT_MAX];
int __attribute__((section(" .cmx_direct.data"))) image_width = 0;
int __attribute__((section(" .cmx_direct.data"))) image_height = 0;

#ifdef TESTIMAGE
u32  __attribute__((section(".ddr_direct.bss"))) g_testimage_num = 0;
u32  __attribute__((section(".ddr_direct.bss"))) g_testimg_incnum = 0;
#endif


u8* usbBufPtr[MAX_USED_BUF] = {usbBuf1, usbBuf2, usbBuf3, usbBuf4};
#define Y_BUFFER_SIZE_MAX   (RES_WIDTH_MAX * RES_HEIGHT_MAX)
#define U_BUFFER_SIZE_MAX   ((RES_WIDTH_MAX * RES_HEIGHT_MAX)/4)
#define V_BUFFER_SIZE_MAX   U_BUFFER_SIZE_MAX
#define PP_BUFS             6
ALIGNED(64) DDR_AREA UInt8      convBuf[PP_BUFS][Y_BUFFER_SIZE_MAX + U_BUFFER_SIZE_MAX + V_BUFFER_SIZE_MAX];
ALIGNED(64) Cam214SIPPDebayerHdmi myPl;

extern u32 deepLearnBegin;

volatile  __attribute__((section(".ddr_direct.bss"))) bool fathomEndFlag = false;
volatile  __attribute__((section(".ddr_direct.bss"))) bool detectDataPreparedFlag = false;
volatile  __attribute__((section(".ddr_direct.bss"))) int indeeplearning = 0;
volatile  __attribute__((section(".ddr_direct.bss"))) bool trackDataPreparedFlag = false;
volatile  __attribute__((section(".ddr_direct.bss"))) bool scoreDataPreparedFlag = false;
volatile  __attribute__((section(".ddr_direct.bss"))) bool detectFlag = false;
volatile  __attribute__((section(".ddr_direct.bss"))) float modeloutscore = 0.0;
volatile  __attribute__((section(".ddr_direct.bss"))) int deeplearningFlag[MAX_GRAPH];
volatile  __attribute__((section(".ddr_direct.bss"))) int deeplearningPhase = EMPTY;
uint8_t __attribute__((section(".ddr_direct.bss"))) g_ucMac[6];
// need to place g_xeye_id to ddr section to make syncronization with OS and RT
char __attribute__((section(".ddr_direct.bss"))) g_xeye_id[16];

SippPipeline* pl;
SippFilter*   dmaIn;
SippFilter*   lut;
SippFilter*   dmaOut;

DmaParam*     dmaInCfg;
DmaParam*     dmaOutCfg;
LutParam*     lutCfg;

volatile u32 frameReady;
volatile u32 testComplete = 0x0;
int g_usbAttached = 0;
uint8_t* g_pucConvUsbBuf;
int32_t g_iUsbLock;
int32_t g_iDataReady;
char firm_version[32];
#ifdef TESTIMAGE
bool end_of_sequence = false;
#endif
char *g_xeye_version = NULL;
XeyeBoardInfo_t xeye_board_info;
// camera mode: 0: sensor mode 1: playback mode
volatile  __attribute__((section(".ddr_direct.data"))) int current_camera_mode = 0;
XeyePcbType __attribute__((section(".ddr_direct.data"))) board_pcb_type = XEYE_20;


void appSippCallback(SippPipeline* pPipeline, eSIPP_PIPELINE_EVENT eEvent,
                     SIPP_PIPELINE_EVENT_DATA* ptEventData) {
    if (eEvent == eSIPP_PIPELINE_FRAME_DONE) {
        testComplete = 1;
    }
}

static void usbAttached(struct leonIPCChannel_t* channel) {
    int status;
    g_usbAttached = 1;
    if (LeonIPCReadMessage(channel, &receivedMessage) == IPC_SUCCESS) {
        printf("usb attached\n");
    }
}


tyTimeStamp mySIPPTimer;
u64 cyclesSIPP;
volatile float sippTimePassed;

// config sipp input and output
void configIOBuffer(Cam214SIPPDebayerHdmi* pVideo, UInt8* inputBuf,
                    UInt8* outputYBuf,
                    UInt8* outputUBuf,
                    UInt8* outputVBuf) {
    DmaParam* dmaInCfg  = (DmaParam*)pVideo->dmaIn0->params;
    DmaParam* dmaOutYCfg = (DmaParam*)pVideo->dmaOutY->params;
    DmaParam* dmaOutUCfg = (DmaParam*)pVideo->dmaOutU->params;
    DmaParam* dmaOutVCfg = (DmaParam*)pVideo->dmaOutV->params;
    //Dma-In params
    dmaInCfg->ddrAddr  = (UInt32)inputBuf;

    //Dma-Out params
    dmaOutYCfg->ddrAddr = (UInt32)outputYBuf;
    dmaOutUCfg->ddrAddr = (UInt32)outputUBuf;
    dmaOutVCfg->ddrAddr = (UInt32)outputVBuf;
}


void configRGBIOBuffer(SippRGB* pVideo, UInt8* inputBuf,
                       UInt8* outputBuf) {
    DmaParam* dmaInCfg  = (DmaParam*)pVideo->dmaIn0->params;
    DmaParam* dmaOutCfg = (DmaParam*)pVideo->dmaOut0->params;
    DmaParam* dmaOut1Cfg = (DmaParam*)pVideo->dmaOut1->params;

    dmaInCfg->ddrAddr  = (UInt32)inputBuf;
    dmaOutCfg->ddrAddr = (UInt32)outputBuf;
    dmaOut1Cfg->ddrAddr = (UInt32)lumaBuf;
}

void initUsbBuf(void) {
    memset(usbBuf1, 0, sizeof(usbBuf1));
    memset(usbBuf2, 0, sizeof(usbBuf2));
    memset(usbBuf3, 0, sizeof(usbBuf3));
    memset(usbBuf4, 0, sizeof(usbBuf4));
}

void YUV420ToYUV422i(UInt8* yIn, UInt8* uIn, UInt8* vIn, UInt8* out, UInt32 yWidth,
                     UInt32 yHeight, UInt32 tWidth, UInt32 tHigh) {
    int i, j = 0;
    int yH = 0;
    int yW = 0;
    int uvH = 0;
    int uvW = 0;
    int uvWidth = yWidth / 2;
    UInt32 out_idx_1 = 0;
    int dhigh = (yHeight - tHigh) / 2;
    int dWidth = (yWidth - tWidth) / 2;
    UInt8* y0;
    UInt8* u0;
    UInt8* v0;

    out_idx_1 += dhigh * yWidth * 2;

    for (i = dhigh; i < (int)tHigh + dhigh; i++) {
        yH = i;
        uvH = i / 2 ;

        out_idx_1 += dWidth * 2;

        y0 = &yIn[yH * yWidth];
        u0 = &uIn[uvH * uvWidth];
        v0 = &vIn[uvH * uvWidth];

        for (j = dWidth; j < (int)tWidth + dWidth; j = j + 2) {
            yW = j ;
            uvW = j / 2;

            out[out_idx_1++] = (UInt8)y0[yW];
            out[out_idx_1++] = (UInt8)u0[uvW];
            out[out_idx_1++] = (UInt8)y0[yW + 1];
            out[out_idx_1++] = (UInt8)v0[uvW];
        }
        out_idx_1 += dWidth * 2;
    }
}

int g_usbAttached;
int g_enable_learning = 1;
int g_save_raw = 0;

// rt running. deepLearnF param is used for c++ calling in c.
int runInit(deepFunc deepLearnF) {
    s32 status;
    camErrorType camStatus;
    s32 boardStatus;
    u32 newFrame = FRAME_READY;
    modelInitDone = 0;
    g_usbAttached = 0;
    memset(deeplearningFlag, 0, sizeof(deeplearningFlag));
    deeplearningPhase = EMPTY;
    fathomEndFlag = false;
    detectFlag = false;
    indeeplearning = 0;
    modeloutscore = 0.0;
    char filename[80];
    int y_buffer_size = 0;
    int u_buffer_size = 0;
    int v_buffer_size = 0;
    LrtSysConfig_t sys_config;
    mvSetHeap((unsigned int)leonHeap, LEON_HEAP_SIZE);

    sys_config.sys_clock_enable = true;
    sys_config.board_init_enable = true;
    leonRT_platform_init(sys_config, &xeye_board_info);
    board_pcb_type = xeye_board_info.hw_pcb_version;
    if (board_pcb_type == XEYE_20) {
        image_width = 1152;
        image_height = 768;
    } else {
        image_width = 1920;
        image_height = 1080;
    }
    y_buffer_size = image_width * image_height;
    u_buffer_size = (image_width * image_height)/4;
    v_buffer_size = u_buffer_size;
    xeye_board_info.camSpec.width = image_width;
    xeye_board_info.camSpec.height = image_height;
    xeye_board_info.camSpec.cam_bpp = 3;        // This cam_bpp is useless
    // Warning: xeye-face+isp need set sensor format but xeyeV2+ar0330 is unnecessary
    // as platform will detect sensor format
    xeye_board_info.sensor_format = ISP_YUV420P;
    g_xeye_version = get_xeye_version();
    memcpy(firm_version, g_xeye_version, strlen(g_xeye_version));

    swcLeonSetPIL(0);

    processingFrameCtr = newCamFrameCtr;
    if (board_pcb_type == XEYE_20) {
        printf("Init Sipp platform..\n");
        sippPlatformInit();

        printf("Build SIPP YUV pipe..\n");
        buildCam214SIPPDebayerHdmi(&myPl);

        printf("Configure  YUV ISP parameters..\n");
        ISPPipeCreateParams();

        printf("Configure YUV SIPP pipe..\n");
        configCam214SIPPDebayerHdmi(&myPl);

        printf("Configure initial sipp buffers \n");
    }
#ifndef TESTIMAGE
    current_camera_mode = 0;
#else
    current_camera_mode = 1;
#endif
    xeye_camera_init((CameraMode_t)current_camera_mode, &xeye_board_info);

    status = LeonIPCTxInit(&LRTtoLOSChannel, messagePool, MSG_QUEUE_SIZE, MSG_SIZE);

    if (status) {
        return status;
    }

    status = LeonIPCRxInit(&LOStoLRTChannel, usbAttached, IRQ_DYNAMIC_4, 5);
    if (status) {
        return status;
    }
        
    XeyeDrvReadMacFromEeprom(xeye_board_info.eeprom_i2c_handle, g_ucMac);
    printf("mac addr: %02x%02x-%02x%02x-%02x%02x", g_ucMac[0], g_ucMac[1], g_ucMac[2],
                                                   g_ucMac[3], g_ucMac[4], g_ucMac[5]);

    printf("xeye camera init finish\n");

    DrvLeonRTSignalBootCompleted();
    initUsbBuf();

    printf("waiting for fathom init model, graph source state=%d\n", modelInitDone);
    while (1) {
        DrvTimerSleepMs(100);
        if(0 != modelInitDone || g_running_mode == LIVE) {
            break;
        }
    }

    printf("initialization is done, graph source state=%d, image process starts:\n", modelInitDone);
    g_pucConvUsbBuf = rgbBuf;
    processingFrameCtr = 0;
#ifdef TESTIMAGE
    g_testimg_incnum = 0;
    while(1) {
        deepLearnF(NULL, NULL, NULL, image_width, image_height);
        if (end_of_sequence) {
            break;
        }
        g_iDataReady = 0;
        LeonIPCSendMessage(&LRTtoLOSChannel, &newFrame);

        g_testimg_incnum++;
    }
#else

    float sensorTime = 0.0;
    int deepCount = 0;
    int sensorFrame = 0;
    if (board_pcb_type == XEYE_20) {
        // TODO(zhoury): check this fucntion
        XeyeDrvAecInit(240, 1, xeye_board_info);

        isp_pipeline_params_update(&myPl, g_ccm_index);
        DrvTimerSleepMs(1500);
    }
    xeye_camera_start((CameraMode_t)current_camera_mode, &xeye_board_info);

    while (1) {
        if (newCamFrameCtr != processingFrameCtr) {
            deepCount++;
            sensorTime += getTime();
            if (newCamFrameCtr < 5)
                continue;
            if (deepCount % 100 == 0) {
                mvLog(MVLOG_DEBUG, "sensor img %d frames/s,deep %d frames/s\n",
                       (int)(1000 / (sensorTime / (newCamFrameCtr - sensorFrame))), 
                       (1000 / (sensorTime / 100)));
                sensorTime = 0.0;
                deepCount = 0;
                sensorFrame = newCamFrameCtr;
            }

            DrvTimerStartTicksCount(&mySIPPTimer);
            u32 newFrame = FRAME_READY;
            processingFrameCtr = newCamFrameCtr;
            if (board_pcb_type == XEYE_20) {
                g_pucConvUsbBuf = &usbBufPtr[(usbFrameCtr + 1) % MAX_USED_BUF][PAYLOAD_HEADER_OFFSET];

                if (g_iUsbLock == 1 || g_iDataReady == 0) {
                    DrvTimerSleepMs(5);
                }

                testComplete = 0;

                g_pucConvBufY = convBuf[((usbFrameCtr) % PP_BUFS)];
                g_pucConvBufU = &convBuf[((usbFrameCtr) % PP_BUFS)][y_buffer_size];
                g_pucConvBufV = &convBuf[((usbFrameCtr) % PP_BUFS)][y_buffer_size + u_buffer_size];

                configIOBuffer(&myPl, RawCamBufPtr[(newCamFrameCtr - 2) % MAX_USED_BUF],
                            g_pucConvBufY, g_pucConvBufU, g_pucConvBufV);

                sippProcessFrame(myPl.pl);

                if (g_save_raw) {
                    sprintf(filename, "%d", newCamFrameCtr - 1);
                    g_save_raw = 0;
                    sippWrFileU8(g_pucConvBufY, y_buffer_size + u_buffer_size + v_buffer_size, filename);
                }

                if (usbFrameCtr % 6 == 0) {
                    XeyeDrvAecSetParameter(120, 130, 100.0f, 32, 240, 720);
                    XeyeDrvStartAec(g_pucConvBufY, image_width, image_height);
                }
            } else {
                g_pucConvBufY = &RawCamBufPtr[(newCamFrameCtr - 2) % MAX_USED_BUF][0];
                g_pucConvBufU = &RawCamBufPtr[(newCamFrameCtr - 2) % MAX_USED_BUF][y_buffer_size];
                g_pucConvBufV = &RawCamBufPtr[(newCamFrameCtr - 2) % MAX_USED_BUF][y_buffer_size + u_buffer_size];
            }
            if (g_enable_learning == 1 && g_running_mode != LIVE) {
                deepLearnF(NULL, NULL, NULL, image_width, image_height);
            }

            if(processingFrameCtr > 10 && g_usbAttached == 1) {
                g_pucConvUsbBuf = &usbBufPtr[(usbFrameCtr + 1) % MAX_USED_BUF][PAYLOAD_HEADER_OFFSET];
                YUV420ToYUV422i(g_pucConvBufY, g_pucConvBufU, g_pucConvBufV, \
                                g_pucConvUsbBuf, image_width, image_height, \
                                image_width, image_height);
                g_iDataReady = 0;
                LeonIPCSendMessage(&LRTtoLOSChannel, &newFrame);
            }
            usbFrameCtr++;
        }
    }
#endif
    return 0;
}

float getTime(void) {
    DrvTimerGetElapsedTicks(&mySIPPTimer, &cyclesSIPP);
    return (float)DrvTimerTicksToMs(cyclesSIPP);
}

void timeprint(char* str) {
    DrvTimerGetElapsedTicks(&mySIPPTimer, &cyclesSIPP);
    sippTimePassed = (float)DrvTimerTicksToMs(cyclesSIPP);
    printf(str);
    printf(": time passed is %f\n", sippTimePassed);
    return;
}

