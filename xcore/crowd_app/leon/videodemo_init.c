///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Application configuration Leon file
///

// 1: Includes
// ----------------------------------------------------------------------------

#include "videodemo.h"
#include "udevice.h"
#include "protovideo.h"
#include "videoclientlib.h"
#include "uplatformapi.h"
#include "usbpumpdebug.h"
#include "usbpumplist.h"
#include "usbpump_types.h"
#include "uisobufhdr.h"
#include "usbpumplib.h"
#include "usbpumpapi.h"
#include "cam_config.h"
#include "mv_types.h"
#include "LeonIPCApi.h"
#include "DrvIcb.h"
#include "OsDrvTimer.h"
#include "usbpump_application_rtems_api.h"
#include "usbpump_devspeed.h"
#include "platform_pubdef.h"
// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------

uint32_t __attribute__((section(".cmx_direct.data"))) receivedMsg[MSG_QUEUE_SIZE* MSG_SIZE];

// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

extern leonIPCChannel_t lrt_LRTtoLOSChannel;
extern leonIPCChannel_t lrt_LOStoLRTChannel;
extern int lrt_image_width;
extern int lrt_image_height;
// Sections decoration is required here for downstream tools
extern u32 lrt_usbFrameCtr;
extern u8* lrt_usbBufPtr[MAX_USED_BUF];

__TMS_TYPE_DEF_STRUCT(UNCOMPRESSED_APP_CONTEXT);

struct __TMS_STRUCTNAME(UNCOMPRESSED_APP_CONTEXT) {
    VIDEODEMO_CONTEXT*     pVideoDemoCtx;
    UINT32            VideoBufSize;

    // Presentation Time Stamp :
    // a source clock time in native device unit
    UINT32            dwPTS;

    // SOF Counter :
    // doesn't need to match the SOF token, Just a 1 kHz counter
    UINT16            wSofCounter;
    UINT8             HeaderToggle;
    UINT32            wVideoByteIndex;
    UINT32            wNumberOfByte;
    UCALLBACKCOMPLETION    WriteFrameCompletion;
};

// 4: Static Local Data
// ----------------------------------------------------------------------------
static uint32_t message;

// Constant data

CONST UPROTO_VIDEO_CONFIG   gk_VideoDemo_ProtoConfig =
    UPROTO_VIDEO_CONFIG_INIT_V1(
        &gk_UsbPumpVideo_InSwitch,
        VIDEODEMO_CONFIG_CONTROLBUFFER_SIZE,
        VIDEODEMO_CONFIG_MAXQUEUED_BUFFER,
        0,  /* no timeout */
        0   /* no timeout */
    );


CONST TEXT* CONST gk_VideoDemo_ErrorName[] = UPROTO_VIDEO_ERROR__INIT;

CONST TEXT* CONST gk_VideoDemo_StatusName[] = UPROTO_VIDEO_STATUS__INIT;


// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
static UPROTO_VIDEO_CONTROL_OPEN_CB_FN    VideoClientLibI_ControlOpen_Callback;
static UPROTO_VIDEO_STREAM_OPEN_CB_FN     VideoClientLibI_StreamOpen_Callback;
static UPROTO_VIDEO_STREAM_GET_INFO_CB_FN VideoClientLibI_StreamGetInfo_Callback;
static UPROTO_VIDEO_STREAM_CLOSE_CB_FN    VideoClientLibI_StreamClose_Done;
static UPROTO_VIDEO_STREAM_WRITE_CB_FN    VideoDemo_WriteOneFrame_Done;

static CALLBACKFN    VideoClientLibI_StreamOpen_Sync;
static CALLBACKFN    VideoDemoI_SendAnotherFrame;

// 6: Functions Implementation
// ----------------------------------------------------------------------------


// Name:    VideoDemo_UncompressedDemoInit()
//
// Function:
//     Application instance initialization function, called by VideoDemo_ClientCreate.
//
// Definition:
//     VIDEODEMO_CONTEXT * VideoDemo_UncompressedDemoInit(
//         UPLATFORM *        pPlatform,
//         USBPUMP_OBJECT_HEADER *    pFunctionObject
//         );
//
// Description:
//     This module will allocate memory needed for application instance, give the
//     initial values to application context variables and send Open command to
//     protocol layer.
//
// Returns:
//     pointer to application context.


VIDEODEMO_CONTEXT* VideoDemo_UncompressedDemoInit(
    UPLATFORM* pPlatform,
    USBPUMP_OBJECT_HEADER* pFunctionObject) {
    PUNCOMPRESSED_APP_CONTEXT        pAppContext;
    VIDEODEMO_CONTEXT*        pVideoDemoCtx;

    UPROTO_VIDEO* CONST        pVideo = __TMS_CONTAINER_OF(
                                            pFunctionObject,
                                            UPROTO_VIDEO,
                                            ObjectHeader
                                        );

    // Allocate videodemo application context
    pAppContext = UsbPumpPlatform_MallocZero(pPlatform, sizeof(*pAppContext));

    if (pAppContext == NULL) {
        TTUSB_OBJPRINTF((&pVideo->ObjectHeader, UDMASK_ERRORS,
                         "?VideoDemo_UncompressedDemoInit:"
                         " pAppContext allocation fail\n"
                        ));

        return NULL;
    }

    // Allocate videodemo context

    pVideoDemoCtx = UsbPumpPlatform_MallocZero(pPlatform, sizeof(*pVideoDemoCtx));

    if (pVideoDemoCtx == NULL) {
        UsbPumpPlatform_Free(
            pPlatform,
            pAppContext,
            sizeof(*pAppContext)
        );

        TTUSB_OBJPRINTF((&pVideo->ObjectHeader, UDMASK_ERRORS,
                         "?VideoDemo_UncompressedDemoInit:"
                         " pVideoDemoCtx allocation fail\n"
                        ));
        return NULL;
    }

    pAppContext->pVideoDemoCtx = pVideoDemoCtx;
    pAppContext->dwPTS = 0;
    pAppContext->wSofCounter = 0;

    pVideoDemoCtx->pVideo = pVideo;
    pVideoDemoCtx->pPlatform = pPlatform;
    pVideoDemoCtx->pAppContext = pAppContext;
    pVideoDemoCtx->hVideoIn = UPROTO_VIDEO_STREAM_INVALID_HANDLE;
    pVideoDemoCtx->hVideoOut = UPROTO_VIDEO_STREAM_INVALID_HANDLE;

    // current setting initialization of Processing Unit
    pVideoDemoCtx->ProcUnitCur.wBackLightCompensation =
        PU_BACKLIGHT_COMPENSATION_CONTROL_DEF;
    pVideoDemoCtx->ProcUnitCur.wBrightness =
        PU_BRIGHTNESS_CONTROL_DEF;
    pVideoDemoCtx->ProcUnitCur.wContrast =
        PU_CONTRAST_CONTROL_DEF;
    pVideoDemoCtx->ProcUnitCur.bPowerLineFrequency =
        PU_POWER_LINE_FREQUENCY_CONTROL_DEF;
    pVideoDemoCtx->ProcUnitCur.wHue =
        PU_HUE_CONTROL_DEF;
    pVideoDemoCtx->ProcUnitCur.wSaturation =
        PU_SATURATION_CONTROL_DEF;
    pVideoDemoCtx->ProcUnitCur.wSharpness =
        PU_SHARPNESS_CONTROL_DEF;
    pVideoDemoCtx->ProcUnitCur.wGamma =
        PU_GAMMA_CONTROL_DEF;
    pVideoDemoCtx->ProcUnitCur.wWhiteBalanceTemperature =
        PU_WHITE_BALANCE_TEMPERATURE_CONTROL_DEF;
    pVideoDemoCtx->ProcUnitCur.bHueAuto =
        PU_HUE_AUTO_CONTROL_DEF;
    pVideoDemoCtx->ProcUnitCur.bWhiteBalanceTemperatureAuto =
        PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL_DEF;

    // current setting initialization of Probe/Commit
    pVideoDemoCtx->ProbeCur.bmHint =
        pVideoDemoCtx->CommitCur.bmHint =
            pVideoDemoCtx->ProbeMin.bmHint =
                pVideoDemoCtx->CommitMin.bmHint =
                    pVideoDemoCtx->ProbeMax.bmHint =
                        pVideoDemoCtx->CommitMax.bmHint =
                            pVideoDemoCtx->ProbeDef.bmHint =
                                pVideoDemoCtx->CommitDef.bmHint =
                                    PROBE_DEFAULT_bmHint;
    pVideoDemoCtx->ProbeCur.bFormatIndex =
        pVideoDemoCtx->CommitCur.bFormatIndex =
            pVideoDemoCtx->ProbeMin.bFormatIndex  =
                pVideoDemoCtx->CommitMin.bFormatIndex =
                    pVideoDemoCtx->ProbeMax.bFormatIndex  =
                        pVideoDemoCtx->CommitMax.bFormatIndex =
                            pVideoDemoCtx->ProbeDef.bFormatIndex  =
                                pVideoDemoCtx->CommitDef.bFormatIndex =
                                    PROBE_DEFAULT_bFormatIndex;
    pVideoDemoCtx->ProbeCur.bFrameIndex =
        pVideoDemoCtx->CommitCur.bFrameIndex =
            pVideoDemoCtx->ProbeMin.bFrameIndex =
                pVideoDemoCtx->CommitMin.bFrameIndex =
                    pVideoDemoCtx->ProbeMax.bFrameIndex =
                        pVideoDemoCtx->CommitMax.bFrameIndex =
                            pVideoDemoCtx->ProbeDef.bFrameIndex =
                                pVideoDemoCtx->CommitDef.bFrameIndex =
                                    PROBE_DEFAULT_bFrameIndex;
    pVideoDemoCtx->ProbeCur.dwFrameInterval =
        pVideoDemoCtx->CommitCur.dwFrameInterval =
            pVideoDemoCtx->ProbeMin.dwFrameInterval =
                pVideoDemoCtx->CommitMin.dwFrameInterval =
                    pVideoDemoCtx->ProbeMax.dwFrameInterval =
                        pVideoDemoCtx->CommitMax.dwFrameInterval =
                            pVideoDemoCtx->ProbeDef.dwFrameInterval =
                                pVideoDemoCtx->CommitDef.dwFrameInterval =
                                    PROBE_DEFAULT_dwFrameInterval;
    pVideoDemoCtx->ProbeCur.wKeyFrameRate =
        pVideoDemoCtx->CommitCur.wKeyFrameRate =
            pVideoDemoCtx->ProbeMin.wKeyFrameRate =
                pVideoDemoCtx->CommitMin.wKeyFrameRate =
                    pVideoDemoCtx->ProbeMax.wKeyFrameRate =
                        pVideoDemoCtx->CommitMax.wKeyFrameRate =
                            pVideoDemoCtx->ProbeDef.wKeyFrameRate =
                                pVideoDemoCtx->CommitDef.wKeyFrameRate =
                                    PROBE_DEFAULT_wKeyFrameRate;
    pVideoDemoCtx->ProbeCur.wPFrameRate =
        pVideoDemoCtx->CommitCur.wPFrameRate =
            pVideoDemoCtx->ProbeMin.wPFrameRate =
                pVideoDemoCtx->CommitMin.wPFrameRate =
                    pVideoDemoCtx->ProbeMax.wPFrameRate =
                        pVideoDemoCtx->CommitMax.wPFrameRate =
                            pVideoDemoCtx->ProbeDef.wPFrameRate =
                                pVideoDemoCtx->CommitDef.wPFrameRate =
                                    PROBE_DEFAULT_wPFrameRate;
    pVideoDemoCtx->ProbeCur.wCompQuality =
        pVideoDemoCtx->CommitCur.wCompQuality =
            pVideoDemoCtx->ProbeMin.wCompQuality =
                pVideoDemoCtx->CommitMin.wCompQuality =
                    pVideoDemoCtx->ProbeMax.wCompQuality =
                        pVideoDemoCtx->CommitMax.wCompQuality =
                            pVideoDemoCtx->ProbeDef.wCompQuality =
                                pVideoDemoCtx->CommitDef.wCompQuality =
                                    PROBE_DEFAULT_wCompQuality;
    pVideoDemoCtx->ProbeCur.wCompWindowSize =
        pVideoDemoCtx->CommitCur.wCompWindowSize =
            pVideoDemoCtx->ProbeMin.wCompWindowSize =
                pVideoDemoCtx->CommitMin.wCompWindowSize =
                    pVideoDemoCtx->ProbeMax.wCompWindowSize =
                        pVideoDemoCtx->CommitMax.wCompWindowSize =
                            pVideoDemoCtx->ProbeDef.wCompWindowSize =
                                pVideoDemoCtx->CommitDef.wCompWindowSize =
                                    PROBE_DEFAULT_wCompWindowSize;
    pVideoDemoCtx->ProbeCur.wDelay =
        pVideoDemoCtx->CommitCur.wDelay =
            pVideoDemoCtx->ProbeMin.wDelay =
                pVideoDemoCtx->CommitMin.wDelay =
                    pVideoDemoCtx->ProbeMax.wDelay =
                        pVideoDemoCtx->CommitMax.wDelay =
                            pVideoDemoCtx->ProbeDef.wDelay =
                                pVideoDemoCtx->CommitDef.wDelay =
                                    PROBE_DEFAULT_wDelay;
    pVideoDemoCtx->ProbeCur.dwMaxVideoFrameSize =
        pVideoDemoCtx->CommitCur.dwMaxVideoFrameSize =
            pVideoDemoCtx->ProbeMin.dwMaxVideoFrameSize =
                pVideoDemoCtx->CommitMin.dwMaxVideoFrameSize =
                    pVideoDemoCtx->ProbeMax.dwMaxVideoFrameSize =
                        pVideoDemoCtx->CommitMax.dwMaxVideoFrameSize =
                            pVideoDemoCtx->ProbeDef.dwMaxVideoFrameSize =
                                pVideoDemoCtx->CommitDef.dwMaxVideoFrameSize =
                                    PROBE_DEFAULT_dwMaxVideoFrameSize;
    pVideoDemoCtx->ProbeCur.dwMaxPayloadTransferSize =
        pVideoDemoCtx->CommitCur.dwMaxPayloadTransferSize =
            pVideoDemoCtx->ProbeMin.dwMaxPayloadTransferSize =
                pVideoDemoCtx->CommitMin.dwMaxPayloadTransferSize =
                    pVideoDemoCtx->ProbeMax.dwMaxPayloadTransferSize =
                        pVideoDemoCtx->CommitMax.dwMaxPayloadTransferSize =
                            pVideoDemoCtx->ProbeDef.dwMaxPayloadTransferSize =
                                pVideoDemoCtx->CommitDef.dwMaxPayloadTransferSize =
                                    PROBE_DEFAULT_dwMaxPayloadTransferSize;
    pVideoDemoCtx->ProbeCur.dwClockFrequency =
        pVideoDemoCtx->CommitCur.dwClockFrequency =
            pVideoDemoCtx->ProbeMin.dwClockFrequency =
                pVideoDemoCtx->CommitMin.dwClockFrequency =
                    pVideoDemoCtx->ProbeMax.dwClockFrequency =
                        pVideoDemoCtx->CommitMax.dwClockFrequency =
                            pVideoDemoCtx->ProbeDef.dwClockFrequency =
                                pVideoDemoCtx->CommitDef.dwClockFrequency =
                                    PROBE_DEFAULT_dwClockFrequency;
    pVideoDemoCtx->ProbeCur.bmFramingInfo =
        pVideoDemoCtx->CommitCur.bmFramingInfo =
            pVideoDemoCtx->ProbeMin.bmFramingInfo =
                pVideoDemoCtx->CommitMin.bmFramingInfo =
                    pVideoDemoCtx->ProbeMax.bmFramingInfo =
                        pVideoDemoCtx->CommitMax.bmFramingInfo =
                            pVideoDemoCtx->ProbeDef.bmFramingInfo =
                                pVideoDemoCtx->CommitDef.bmFramingInfo =
                                    PROBE_DEFAULT_bmFramingInfo;
    pVideoDemoCtx->ProbeCur.bPreferedVersion =
        pVideoDemoCtx->CommitCur.bPreferedVersion =
            pVideoDemoCtx->ProbeMin.bPreferedVersion =
                pVideoDemoCtx->CommitMin.bPreferedVersion =
                    pVideoDemoCtx->ProbeMax.bPreferedVersion =
                        pVideoDemoCtx->CommitMax.bPreferedVersion =
                            pVideoDemoCtx->ProbeDef.bPreferedVersion =
                                pVideoDemoCtx->CommitDef.bPreferedVersion =
                                    PROBE_DEFAULT_bPreferedVersion;
    pVideoDemoCtx->ProbeCur.bMinVersion =
        pVideoDemoCtx->CommitCur.bMinVersion =
            pVideoDemoCtx->ProbeMin.bMinVersion =
                pVideoDemoCtx->CommitMin.bMinVersion =
                    pVideoDemoCtx->ProbeMax.bMinVersion =
                        pVideoDemoCtx->CommitMax.bMinVersion =
                            pVideoDemoCtx->ProbeDef.bMinVersion =
                                pVideoDemoCtx->CommitDef.bMinVersion =
                                    PROBE_DEFAULT_bMinVersion;
    pVideoDemoCtx->ProbeCur.bMaxVersion =
        pVideoDemoCtx->CommitCur.bMaxVersion =
            pVideoDemoCtx->ProbeMin.bMaxVersion =
                pVideoDemoCtx->CommitMin.bMaxVersion =
                    pVideoDemoCtx->ProbeMax.bMaxVersion =
                        pVideoDemoCtx->CommitMax.bMaxVersion =
                            pVideoDemoCtx->ProbeDef.bMaxVersion =
                                pVideoDemoCtx->CommitDef.bMaxVersion =
                                    PROBE_DEFAULT_bMaxVersion;

    // current setting initialization of Still Probe/Commit
    pVideoDemoCtx->StillProbeCur.bFormatIndex =
        pVideoDemoCtx->StillCommitCur.bFormatIndex =
            STILL_PROBE_DEFAULT_bFormatIndex;
    pVideoDemoCtx->StillProbeCur.bFrameIndex =
        pVideoDemoCtx->StillCommitCur.bFrameIndex =
            STILL_PROBE_DEFAULT_bFrameIndex;
    pVideoDemoCtx->StillProbeCur.bCompressionIndex =
        pVideoDemoCtx->StillCommitCur.bCompressionIndex =
            STILL_PROBE_DEFAULT_bCompressionIndex;
    pVideoDemoCtx->StillProbeCur.dwMaxVideoFrameSize =
        pVideoDemoCtx->StillCommitCur.dwMaxVideoFrameSize =
            STILL_PROBE_DEFAULT_dwMaxVideoFrameSize;
    pVideoDemoCtx->StillProbeCur.dwMaxPayloadTransferSize =
        pVideoDemoCtx->StillCommitCur.dwMaxPayloadTransferSize =
            STILL_PROBE_DEFAULT_dwMaxPayLoadTransferSize;

    pVideoDemoCtx->bStillTrigger =     STILL_IMAGE_NORMAL_OPERATION;
    pVideoDemoCtx->nVideoFrameIndex = START_VIDEO_FRAME_INDEX;

    // initialize callback completion
    USBPUMP_CALLBACKCOMPLETION_INIT(
        &pVideoDemoCtx->InitCompletion,
        VideoClientLibI_StreamOpen_Sync,
        NULL
    );

    USBPUMP_CALLBACKCOMPLETION_INIT(
        &pAppContext->WriteFrameCompletion,
        VideoDemoI_SendAnotherFrame,
        NULL
    );

    int status = LeonIPCRxInit(&lrt_LRTtoLOSChannel, NULL, IRQ_DYNAMIC_5, 5);

    if (status) {
        printf("Could not initialize Leon IPC Rx. Error: %d\n", status);
    }

    // Open video control
    VideoClientLib_ControlOpen(
        pVideoDemoCtx->pVideo,
        VideoClientLibI_ControlOpen_Callback,
        pAppContext,
        pVideoDemoCtx,
        &gk_VideoDemo_OutSwitch
    );

    return pVideoDemoCtx;
}

static void VideoClientLibI_ControlOpen_Callback(void*     pCallbackCtx, UINT32 ErrorCode) {
    PUNCOMPRESSED_APP_CONTEXT CONST    pAppContext = pCallbackCtx;
    VIDEODEMO_CONTEXT* CONST    pVideoDemoCtx = pAppContext->pVideoDemoCtx;

    if (ErrorCode != UPROTO_VIDEO_ERROR_OK) {
        TTUSB_OBJPRINTF((&pVideoDemoCtx->pVideo->ObjectHeader,
                         UDMASK_ERRORS,
                         "?VideoClientLib_ControlOpen_Sync:"
                         " ControlOpen() fail (%s)\n",
                         gk_VideoDemo_ErrorName[ErrorCode]
                        ));
    } else {
        UsbPumpPlatform_PostIfNotBusy(
            pVideoDemoCtx->pPlatform,
            &pVideoDemoCtx->InitCompletion,
            pAppContext
        );
    }
}

static void VideoClientLibI_StreamOpen_Sync(void* pClientContext) {
    PUNCOMPRESSED_APP_CONTEXT CONST    pAppContext = pClientContext;
    VIDEODEMO_CONTEXT*     pVideoDemoCtx = pAppContext->pVideoDemoCtx;
    // Open video output stream
    VideoClientLib_StreamOpen(
        pVideoDemoCtx->pVideo,
        VideoClientLibI_StreamOpen_Callback,
        pAppContext,
        (INT8)(-1),    // InterfaceNumber
        (INT8)(-1),    // AlternateSetting
        (INT8)(-1),    // TerminalLink
        FALSE        // fTrueIfVideoInput
    );
}

static void VideoClientLibI_StreamOpen_Callback(
    void*     pCallbackCtx,
    UINT32    ErrorCode,
    UPROTO_VIDEO_STREAM_HANDLE    hVideoStream
) {
    PUNCOMPRESSED_APP_CONTEXT CONST    pAppContext = pCallbackCtx;
    VIDEODEMO_CONTEXT*         pVideoDemoCtx = pAppContext->pVideoDemoCtx;

    if (ErrorCode != UPROTO_VIDEO_ERROR_OK) {
        TTUSB_OBJPRINTF((
                            &pVideoDemoCtx->pVideo->ObjectHeader,
                            UDMASK_ERRORS,
                            "?VideoClientLibI_StreamOpen_Callback:"
                            " StreamOpen() fail (%s)\n",
                            gk_VideoDemo_ErrorName[ErrorCode]
                        ));
    } else {
        pVideoDemoCtx->hVideoOut = hVideoStream;
    }
}

void VideoDemo_Start(VIDEODEMO_CONTEXT*     pVideoDemoCtx) {
    VideoClientLib_StreamGetInfo(
        pVideoDemoCtx->pVideo,
        VideoClientLibI_StreamGetInfo_Callback,
        pVideoDemoCtx->pAppContext,
        pVideoDemoCtx->hVideoOut
    );
}

static void VideoClientLibI_StreamGetInfo_Callback(
    void*     pCallbackCtx,
    UINT32    ErrorCode,
    UINT8     bInterfaceNumber,
    UINT8     bAlternateSetting,
    UINT8     bTerminalLink,
    UINT16    wMaxPacketSize,
    UINT16    wTransportHeaderSize
) {
    (void) wMaxPacketSize;// "use" the variables to hush the compiler warning.
    (void) wTransportHeaderSize;// "use" the variables to hush the compiler warning.

    PUNCOMPRESSED_APP_CONTEXT CONST    pAppContext = pCallbackCtx;
    VIDEODEMO_CONTEXT* CONST    pVideoDemoCtx = pAppContext->pVideoDemoCtx;
    UPROTO_VIDEO* CONST        pVideo = pAppContext->pVideoDemoCtx->pVideo;
    BOOL fResult;

    USBPUMP_TRACE_PARAMETER(pVideo);
    USBPUMP_UNREFERENCED_PARAMETER(bInterfaceNumber);
    USBPUMP_UNREFERENCED_PARAMETER(bAlternateSetting);
    USBPUMP_UNREFERENCED_PARAMETER(bTerminalLink);

    fResult = TRUE;

    if (ErrorCode != UPROTO_VIDEO_ERROR_OK) {
        TTUSB_OBJPRINTF((
                            &pVideo->ObjectHeader,
                            UDMASK_ERRORS,
                            "?VideoClientLi_StreamGetInfo_Callback:"
                            " GetStreamInfo() fail (%s)\n",
                            gk_VideoDemo_ErrorName[ErrorCode]
                        ));

        fResult = FALSE;
    }

    if (fResult) {
        pAppContext->HeaderToggle    = 0;
        pAppContext->wVideoByteIndex = 0;
        pAppContext->wNumberOfByte   = 0;

        VideoDemo_WriteOneFrame_Done(
            pVideoDemoCtx,
            UPROTO_VIDEO_ERROR_OK,
            NULL,
            0
        );
        UDEVICE* pDevice = UsbPump_Rtems_DataPump_GetDevice();

        if (USBPUMP_IS_SUPER_SPEED(pDevice->udev_bCurrentSpeed)) {
            message = USB_SUPER_SPEED;
        } else {
            message = USB_HIGH_SPEED;
        }
        printf("********************************usb insert\n");
        // send a meesage to LRT with USB speed information
        LeonIPCSendMessage(&lrt_LOStoLRTChannel, &message);
    } else {
        if (pVideoDemoCtx->hVideoOut !=
                UPROTO_VIDEO_STREAM_INVALID_HANDLE) {
            VideoClientLib_StreamClose(
                pVideoDemoCtx->pVideo,
                VideoClientLibI_StreamClose_Done,
                pVideoDemoCtx,
                pVideoDemoCtx->hVideoOut
            );
        }

        if (pVideoDemoCtx) {
            UsbPumpPlatform_Free(
                pVideoDemoCtx->pPlatform,
                pVideoDemoCtx,
                sizeof(*pVideoDemoCtx)
            );
        }
    }
}

static void VideoClientLibI_StreamClose_Done(void*   pClientContext, UINT32 ErrorCode) {
    USBPUMP_UNREFERENCED_PARAMETER(pClientContext);
    USBPUMP_UNREFERENCED_PARAMETER(ErrorCode);
}

// Name:    VideoDemo_WriteOneFrame()
//
// Function:
//     Send one frame video data to host side.
//
// Definition:
//     VOID VideoDemo_WriteOneFrame(
//         VIDEODEMO_CONTEXT *    pVideoDemoCtx,
//         UINT8 *        pBuffer,
//         UINT32         wNumberOfByte,
//         UINT32      dwPTS,
//         UINT16      wSofCounter,
//        );
//
// Description:
//     Process the write data, including adding header fields and copy the
//     data to write buffer, and call the underlying protocol layer to complete
//     the transfer. This routine should be called by client layer periodically.
//
// Returns:
//     Nothing
//
// Notes:
//     The callback function should visit this function again if the transfer
//     size is larger than our buffer size.
void VideoDemo_WriteOneFrame(
    VIDEODEMO_CONTEXT*     pVideoDemoCtx,
    u8* pBuffer,
    u32 wNumberOfByte,
    u32 dwPTS,
    u16 wSofCounter
) {
    PUNCOMPRESSED_APP_CONTEXT CONST pAppContext = pVideoDemoCtx->pAppContext;
    BOOL fEndofFrame = TRUE;
    pAppContext->wVideoByteIndex = 0;
    pAppContext->dwPTS = dwPTS;
    pAppContext->wSofCounter = wSofCounter;
    pAppContext->wNumberOfByte = wNumberOfByte;

    if (pAppContext->HeaderToggle == 0) {
        pAppContext->HeaderToggle = 1;
    } else {
        pAppContext->HeaderToggle = 0;
    }

    pVideoDemoCtx->nWriteBufferHdr = 1;

    // Process Header Fields
    *(pBuffer + UNCOMPRESSED_HDR_HLE_OFFSET) = PAYLOAD_HEADER_SIZE;

    *(pBuffer + UNCOMPRESSED_HDR_BFH_OFFSET) =
        UNCOMPRESSED_HDR_PTS +
        UNCOMPRESSED_HDR_SCR +
        pAppContext->HeaderToggle;

    *(pBuffer + UNCOMPRESSED_HDR_BFH_OFFSET) += UNCOMPRESSED_HDR_EOF;

    UHIL_LE_PUTUINT32(pBuffer + UNCOMPRESSED_HDR_PTS_OFFSET, pAppContext->dwPTS);
    UHIL_LE_PUTUINT32(pBuffer + UNCOMPRESSED_HDR_SCR_STC_OFFSET, pAppContext->dwPTS);
    UHIL_LE_PUTUINT16(pBuffer + UNCOMPRESSED_HDR_SCR_SOF_OFFSET, pAppContext->wSofCounter);
    VideoClientLib_StreamWrite(
        pVideoDemoCtx->pVideo,
        VideoDemo_WriteOneFrame_Done,
        pVideoDemoCtx,
        pVideoDemoCtx->hVideoOut,
        pBuffer,
        wNumberOfByte,
        fEndofFrame
    );
}

// Name:    VideoDemo_WriteOneFrame_Done()
//
// Function:
//     The callback function for VideoDemo_WriteOneFrame()
//
// Definition:
//     VOID  VideoDemo_WriteOneFrame_Done(
//        VOID *        pContext,
//        UINT32        ErrorCode,
//        UINT8 *     pData,
//        BYTES        nData,
//        );
//
// Description:
//     The callback function for VideoDemo_WriteOneFrame()
//
// Returns:
//     Nothing
//
// Notes:
//     This callback function should visit VideoDemo_WriteOneFrame() again
//     if the transfer size     is larger than our buffer size.
extern int32_t lrt_g_iUsbLock;
extern int32_t lrt_g_iDataReady;
static void VideoDemo_WriteOneFrame_Done(
    void*   pContext,
    UINT32  ErrorCode,
    UINT8*  pData,
    BYTES   nData
) {
    VIDEODEMO_CONTEXT* CONST pVideoDemoCtx = pContext;
    PUNCOMPRESSED_APP_CONTEXT CONST pAppContext = pVideoDemoCtx->pAppContext;

    USBPUMP_UNREFERENCED_PARAMETER(nData);
    USBPUMP_UNREFERENCED_PARAMETER(pData);

    if (ErrorCode != UPROTO_VIDEO_ERROR_OK) {
        TTUSB_OBJPRINTF((&pVideoDemoCtx->pVideo->ObjectHeader,
                         UDMASK_ERRORS,
                         "?VideoDemo_WriteOneFrame_Done():"
                         " write fail %s, continue read\n",
                         gk_VideoDemo_ErrorName[ErrorCode]
                        ));
    }

    // Sending a new Frame.
    if (pVideoDemoCtx->fOutputActivate) {
        UsbPumpPlatform_PostIfNotBusy(
            pVideoDemoCtx->pPlatform,
            &pAppContext->WriteFrameCompletion,
            pAppContext
        );
    }
	lrt_g_iUsbLock = 0;
}


extern  void addOutFace2Frame(u8 *img);
static void VideoDemoI_SendAnotherFrame(void* pClientContext) {
    u32 msg;
    PUNCOMPRESSED_APP_CONTEXT CONST     pAppContext = pClientContext;
    VIDEODEMO_CONTEXT* CONST     pVideoDemoCtx = pAppContext->pVideoDemoCtx;

    UsbPumpPlatform_MarkCompletionNotBusy(
        pVideoDemoCtx->pPlatform,
        &pAppContext->WriteFrameCompletion
    );

    pAppContext->dwPTS += 1;
    pAppContext->wSofCounter += 1;
    // PAYLOAD_HEADER_OFFSET is greater than PAYLOAD_HEADER_SIZE to keep the buffers coming
    // after the header properly aligned
    LeonIPCWaitMessage(&lrt_LRTtoLOSChannel, IPC_WAIT_FOREVER);
    LeonIPCReadMessage(&lrt_LRTtoLOSChannel, &msg);
	lrt_g_iUsbLock = 1;
	lrt_g_iDataReady = 1;
    VideoDemo_WriteOneFrame(
        pVideoDemoCtx,
        lrt_usbBufPtr[(lrt_usbFrameCtr -2)% MAX_USED_BUF] + PAYLOAD_HEADER_OFFSET - PAYLOAD_HEADER_SIZE,
        lrt_image_width * lrt_image_height * 2 + PAYLOAD_HEADER_SIZE,
        pAppContext->dwPTS,
        pAppContext->wSofCounter
    );

    pVideoDemoCtx->nVideoFrameIndex = 1;
}
