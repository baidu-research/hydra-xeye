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
#include <string.h>
#include <errno.h>
#include <rtems.h>
#include <DrvGpio.h>
#include "xeye_info.h"
#include "CamGenericApi.h"
#include "sensor_common.h"
#include "xeye_board_init.h"
#include "platform_sensor_api.h"
#include "xeye_info.h"
#include "platform_pubdef.h"
#define MVLOG_UNIT_NAME platform_sensor_api
#include <mvLog.h>

#define USE_MALLOC

#ifdef HUATU_PROJECT
#undef USE_MALLOC
#endif


#ifndef USE_MALLOC
#ifndef USB_IMAGE_WIDTH
#define USB_IMAGE_WIDTH 1920
#endif

#ifndef USB_IMAGE_HEIGHT
#define USB_IMAGE_HEIGHT 1080
#endif

u8 DDR_AREA RawCamBuf[MAX_USED_BUF][USB_IMAGE_WIDTH * USB_IMAGE_HEIGHT * 3] ALIGNED(8);
#endif
u8* __attribute__((section(".ddr_direct.data"))) RawCamBufPtr[MAX_USED_BUF] = {NULL};
volatile ALIGNED(4) uint32_t __attribute__((section(".ddr_direct.data"))) newCamFrameCtr = FIRST_INCOMING_BUF_ID;
volatile ALIGNED(4) uint32_t processingFrameCtr = FIRST_INCOMING_BUF_ID;

GenericCameraHandle          camHndl;
frameSpec                    camFrameSpec;
frameBuffer                  camFrame[MAX_USED_BUF];
static char* sensor_type_name[] = {"Sensor_IMX378", "Sensor_IMX214", "Sensor_AR0330", "Sensor_AR0144",\
                                    "HiSilicon_ISP", "Xchip_ISP", "Nextchip_ISP"};
void prepare_frame_buf(XeyeBoardInfo_t *xeye_board_info) {
    for (int ibuf = 0; ibuf < MAX_USED_BUF; ibuf++) {
        camFrame[ibuf].spec  = camFrameSpec;
        camFrame[ibuf].p1    = (unsigned char*)&RawCamBufPtr[ibuf][0];
        if (xeye_board_info->sensor_format == ISP_YUV420P ) {
            camFrame[ibuf].p2    = &RawCamBufPtr[ibuf][xeye_board_info->camSpec.width * xeye_board_info->camSpec.height];
            camFrame[ibuf].p3    =  camFrame[ibuf].p2 + xeye_board_info->camSpec.width * xeye_board_info->camSpec.height / 4;
        } else {
          camFrame[ibuf].p2    = NULL;
          camFrame[ibuf].p3    = NULL;           
        }
    }
}

bool init_cam_buf(XeyeBoardInfo_t* xeye_board_info) {
    #ifdef USE_MALLOC
    // use malloc to alloc memory
    uint32_t buf_len = xeye_board_info->camSpec.width * \
                       xeye_board_info->camSpec.height *  xeye_board_info->camSpec.cam_bpp;
    for (int i = 0; i < MAX_USED_BUF; i++) {
        // malloc buffer and clear to zero
        RawCamBufPtr[i] = (uint8_t *)calloc(buf_len, sizeof(uint8_t));
        if(RawCamBufPtr[i] == NULL) {
            mvLog(MVLOG_ERROR, "The %d calloc cam buffer failed %08x---%d", i, RawCamBufPtr[i], buf_len);
            return false;
        }

    }
    #else
    // static memory
    for (int i = 0; i < MAX_USED_BUF; ++i) {
        RawCamBufPtr[i] = RawCamBuf[i];
        memset(RawCamBuf[i], 0, USB_IMAGE_WIDTH * USB_IMAGE_HEIGHT * 3);
    }
    #endif
    return true;
}

#ifdef USE_MALLOC
void free_cam_buf(void) {
    for (int i = 0; i < MAX_USED_BUF; i++) {
        if (RawCamBufPtr[i] != NULL)
            free(RawCamBufPtr[i]);
    }
}
#endif

void prepare_frame_spec(XeyeBoardInfo_t* xeye_board_info, CamUserSpec* userCamConfig) {

    camFrameSpec.width  = xeye_board_info->camSpec.width;
    camFrameSpec.height = xeye_board_info->camSpec.height;
    camFrameSpec.bytesPP = xeye_board_info->camSpec.cam_bpp;
    camFrameSpec.stride  = camFrameSpec.width * xeye_board_info->camSpec.cam_bpp;

    if (xeye_board_info->sensor_format == ISP_RGB888) {
        camFrameSpec.type    = RGB888;
    } else if (xeye_board_info->sensor_format == ISP_YUV422){
        camFrameSpec.type    = YUV422i;
    } else if (xeye_board_info->sensor_format == Sensor_RAW16) {
        camFrameSpec.type    = RAW16;
    } else if (xeye_board_info->sensor_format == ISP_YUV420P) {
        camFrameSpec.type    = YUV420p;
    }
		
    if (xeye_board_info->hw_pcb_version == XEYE_20) {
        if (xeye_board_info->sensor_type == Sensor_AR0330 || \
            xeye_board_info->sensor_type == Sensor_AR0144) {
            userCamConfig->mipiControllerNb  = MIPI_CTRL_0;
            userCamConfig->receiverId        = CIF_DEVICE0;
            userCamConfig->sensorResetPin    = XEYE_MIPI0_RST_GPIO;
        } else if ((xeye_board_info->sensor_type == HiSilicon_ISP) || \
                    (xeye_board_info->sensor_type == Nextchip_ISP)) {
            userCamConfig->mipiControllerNb  = MIPI_CTRL_2;
            userCamConfig->receiverId        = SIPP_DEVICE1;
            userCamConfig->sensorResetPin    = XEYE_MIPI1_RST_GPIO;
        } else {
            mvLog(MVLOG_ERROR, "unkown sensor_type on xeye2.0 board");
        }
    } else {
        if ((xeye_board_info->sensor_type == HiSilicon_ISP) || \
            (xeye_board_info->sensor_type == Nextchip_ISP)  || \
            (xeye_board_info->sensor_type == Sensor_AR0330) || \
            (xeye_board_info->sensor_type == Sensor_AR0144)) {
            userCamConfig->mipiControllerNb  = MIPI_CTRL_0;
            userCamConfig->receiverId        = CIF_DEVICE0;
            userCamConfig->sensorResetPin    = XEYE_MIPI0_RST_GPIO;
        } else {
            mvLog(MVLOG_ERROR, "unkown sensor_type on xeye-face board");
        }
    }

    userCamConfig->stereoPairIndex   = CAM_A_ADDR;
    userCamConfig->windowColumnStart = CAM_WINDOW_START_COLUMN;
    userCamConfig->windowRowStart    = CAM_WINDOW_START_ROW;
    userCamConfig->windowWidth       = xeye_board_info->camSpec.width;
    userCamConfig->windowHeight      = xeye_board_info->camSpec.height;
    userCamConfig->generateSync      = NULL;
}

frameBuffer* allocate_next_cam_framebuf(void) {
    ++newCamFrameCtr;
    // TODO(zhoury): this will make image fliker
    // In case of buffer overwrite, skip the current processing buffer by Leon RT
    // if( (newCamFrameCtr - processingFrameCtr) % MAX_USED_BUF == 0 ) {
        //  ++newCamFrameCtr;
    // }
    return (&camFrame[newCamFrameCtr % MAX_USED_BUF]);
}

bool xeye_camera_init(CameraMode_t cam_mode, XeyeBoardInfo_t* xeye_board_info) {
    int status;
    camErrorType camStatus;
    SensorType_t xeye_sensor = Sensor_AR0330;
    GenericCamSpec* staticCamConfig_SS = NULL;
    callbacksListStruct          callbacks = {0};
    interruptsCallbacksListType  isrCallbacks = {0};
    CamUserSpec                  userCamConfig;
    if (cam_mode == SENOSR_MODE) {
        staticCamConfig_SS = read_cam_config(xeye_board_info);
        if(staticCamConfig_SS == NULL) {
            mvLog(MVLOG_ERROR, "staticCamConfig_SS is null");
            return false;
        }

        sensor_hw_reset(xeye_board_info->sensor_type, xeye_board_info->hw_pcb_version);

        xeye_sensor = xeye_board_info->sensor_type;
        mvLog(MVLOG_INFO, "auto check Xeye Sensor: %s mipi channel: %d", sensor_type_name[xeye_board_info->sensor_type], \
                            xeye_board_info->mipi_channel);

        init_cam_buf(xeye_board_info);
        prepare_frame_spec(xeye_board_info, &userCamConfig);
        prepare_frame_buf(xeye_board_info);

        mvLog(MVLOG_INFO, "Configuring  camera and datapath");
        //start camera
        isrCallbacks.getBlock     = NULL;
        isrCallbacks.getFrame     = allocate_next_cam_framebuf;
        isrCallbacks.notification = NULL;
        callbacks.isrCbfList      = &isrCallbacks;
        callbacks.sensorCbfList   = NULL;

        processingFrameCtr = newCamFrameCtr;

        sensor_soft_reset(xeye_board_info);

        camStatus = CamInit(&camHndl, staticCamConfig_SS, &userCamConfig, &callbacks, xeye_board_info->camera_i2c_handle);

        if (camStatus != CAM_SUCCESS) {
            mvLog(MVLOG_ERROR, "Camera Init configuration failed (%d).", camStatus);
            return false;
        }
        mvLog(MVLOG_INFO, "xeye camera init with sensor mode success");
    } else if (cam_mode == USB_PLAYBACK_MODE) {
        mvLog(MVLOG_INFO, "xeye camera init with usb playback mode");
        xeye_board_info->camSpec.width = 1920;
        xeye_board_info->camSpec.height = 1080;
        xeye_board_info->camSpec.cam_bpp = 3;
        xeye_board_info->sensor_format = ISP_RGB888;
        xeye_board_info->sensor_type = HiSilicon_ISP;
        init_cam_buf(xeye_board_info);
        prepare_frame_spec(xeye_board_info, &userCamConfig);
        prepare_frame_buf(xeye_board_info);
        mvLog(MVLOG_INFO, "xeye camera init with usb playback mode success");
    }
    return true;
}

// platform2.0 camera start
bool xeye_camera_start(CameraMode_t cam_mode, XeyeBoardInfo_t* xeye_board_info) {
    camErrorType camStatus;

    if (cam_mode == USB_PLAYBACK_MODE) {
        // playback mode will return success
        return true;
    }
    camStatus  = CamStart(&camHndl);

    if (camStatus != CAM_SUCCESS) {
        mvLog(MVLOG_ERROR, "Camera failed to start (%d).", camStatus);
        return false;
    }
    // TODO(hyx): hardware reset here is only for nextchip-isp, do not hardware reset
    // with other sensor type
    if (xeye_board_info->sensor_type == Nextchip_ISP) {
        sensor_hw_reset(xeye_board_info->sensor_type, xeye_board_info->hw_pcb_version);
        mipi_stream_switch_format(xeye_board_info->camera_i2c_handle, xeye_board_info->sensor_format);
    }

    return true;
}

// platform2.0 camera stop
bool xeye_camera_stop(CameraMode_t cam_mode) {
    camErrorType camStatus;

    if (cam_mode == USB_PLAYBACK_MODE) {
        // playback mode will return success
        return true;
    }
    camStatus = CamStop(&camHndl);
    if (camStatus != CAM_SUCCESS) {
        mvLog(MVLOG_ERROR, "Could not stop the camera(%d).", camStatus);
        return false;
    }

    #ifdef USE_MALLOC
    free_cam_buf();
    #endif
    return true;
}
