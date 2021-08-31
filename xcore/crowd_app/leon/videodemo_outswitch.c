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

#include "protovideo.h"
#include "videoclientlib.h"
#include "usbpumpdebug.h"
#include "usbvideo11.h"
#include "uplatformapi.h"
#include "usbpumplib.h"
#include <stdio.h>
static CALLBACKFN    VideoDemoI_ProcessingUnitEvent_Sync;
static UPROTO_VIDEO_CONTROL_REQUEST_REPLY_CB_FN    VideoDemoI_ControlRequest_Done;

static CALLBACKFN    VideoDemoI_StreamInterfaceEvent_Sync;
static UPROTO_VIDEO_STREAM_REQUEST_REPLY_CB_FN    VideoDemoI_StreamRequest_Done;

// These are the functions that underlying proto (video class) will call in case
// a status change (reset, attached...) or a host request happens.
CONST UPROTO_VIDEO_OUTSWITCH    gk_VideoDemo_OutSwitch =
    UPROTO_VIDEO_OUTSWITCH_INIT_V1(\
                                   VideoDemo_ControlStatusEvent,           \
                                   VideoDemo_ProcessingUnitEvent,          \
                                   VideoDemo_StreamStatusEvent,            \
                                   VideoDemo_StreamInterfaceEvent          \
                                  );


// Name:    VideoDemo_ControlStatusEvent()
//
// Function:
//     Outswitch function of protocol layer for the control interface.
//
// Definition:
//    VOID VideoDemo_ControlStatusEvent(
//        VOID *            pClientContext,
//        UPROTO_VIDEO_STATUS    VideoStatusCode
//        );
//
// Description:
//     The protocol layer will call this outswitch function to inform application
//     layer when status changes, for example attach, detach.
//
// Returns:
//     Nothing

void VideoDemo_ControlStatusEvent(void* pClientContext, UPROTO_VIDEO_STATUS    VideoStatusCode) {
    VIDEODEMO_CONTEXT* CONST    pVideoDemoCtx = pClientContext;
    UPROTO_VIDEO* CONST        pVideo = pVideoDemoCtx->pVideo;

    USBPUMP_TRACE_PARAMETER(pVideo);
    USBPUMP_TRACE_PARAMETER(VideoStatusCode);
    
    printf(" VideoDemo_StatusChangeEvent: %s event\n",gk_VideoDemo_StatusName[VideoStatusCode]);
    TTUSB_OBJPRINTF((&pVideo->ObjectHeader, UDMASK_PROTO,
                     " VideoDemo_StatusChangeEvent: %s event\n",
                     gk_VideoDemo_StatusName[VideoStatusCode]));
}

// Name:    VideoDemo_ProcessingUnitEvent()
//
// Function:
//     Outswitch function of protocol layer to inform Processing Unit of related
//     video request.
//
// Definition:
//     VOID VideoDemo_ProcessingUnitEvent(
//        VOID *        pClientContext,
//        UPROTO_VIDEO_CONTROL_REQUEST *    pRequest
//        );
//
// Description:
//     The protocol layer will call this outswitch function when host issues any
//     request to the Processing Unit.
//
// Returns:
//     Nothing

void VideoDemo_ProcessingUnitEvent(void* pClientContext, UPROTO_VIDEO_CONTROL_REQUEST* pRequest) {
    VIDEODEMO_CONTEXT* CONST pVideoDemoCtx = pClientContext;

    UHIL_cpybuf(&pVideoDemoCtx->ControlRequest, pRequest, sizeof(*pRequest));

    USBPUMP_CALLBACKCOMPLETION_INIT(
        &pVideoDemoCtx->SetupCompletion,
        VideoDemoI_ProcessingUnitEvent_Sync,
        NULL);

    UsbPumpPlatform_PostIfNotBusy(
        pVideoDemoCtx->pPlatform,
        &pVideoDemoCtx->SetupCompletion,
        pVideoDemoCtx);
}

static void VideoDemoI_ProcessingUnitEvent_Sync(void* pClientContext) {
    VIDEODEMO_CONTEXT* CONST pVideoDemoCtx = pClientContext;
    UPROTO_VIDEO_CONTROL_REQUEST* pRequest = &pVideoDemoCtx->ControlRequest;

    UsbPumpPlatform_MarkCompletionNotBusy(pVideoDemoCtx->pPlatform,
                                          &pVideoDemoCtx->SetupCompletion);

    pRequest->Hdr.bAcceptRequest = TRUE;

    switch (pRequest->Hdr.bControlSelector) {
    case USBVIDEO_bControlSelector_PU_BACKLIGHT_COMPENSATION_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->Backlight.wBacklightCompensation =
                PU_BACKLIGHT_COMPENSATION_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->Backlight.wBacklightCompensation =
                PU_BACKLIGHT_COMPENSATION_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->Backlight.wBacklightCompensation =
                PU_BACKLIGHT_COMPENSATION_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->Backlight.wBacklightCompensation =
                PU_BACKLIGHT_COMPENSATION_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->Backlight.wBacklightCompensation =
                PU_BACKLIGHT_COMPENSATION_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->Backlight.wBacklightCompensation =
                pVideoDemoCtx->ProcUnitCur.wBackLightCompensation;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accept all setting from host
            pVideoDemoCtx->ProcUnitCur.wBackLightCompensation =
                pRequest->Backlight.wBacklightCompensation;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_PU_BRIGHTNESS_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->Brightness.wBrightness = PU_BRIGHTNESS_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->Brightness.wBrightness = PU_BRIGHTNESS_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->Brightness.wBrightness = PU_BRIGHTNESS_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->Brightness.wBrightness = PU_BRIGHTNESS_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->Brightness.wBrightness = PU_BRIGHTNESS_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->Brightness.wBrightness =
                pVideoDemoCtx->ProcUnitCur.wBrightness;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accept all setting from host
            pVideoDemoCtx->ProcUnitCur.wBrightness =
                pRequest->Brightness.wBrightness;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_PU_CONTRAST_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->Contrast.wContrast = PU_CONTRAST_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->Contrast.wContrast = PU_CONTRAST_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->Contrast.wContrast = PU_CONTRAST_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->Contrast.wContrast = PU_CONTRAST_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->Contrast.wContrast = PU_CONTRAST_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->Contrast.wContrast = pVideoDemoCtx->ProcUnitCur.wContrast;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accpet all setting from host
            pVideoDemoCtx->ProcUnitCur.wContrast = pRequest->Contrast.wContrast;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_PU_POWER_LINE_FREQUENCY_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->PowerLine.bPowerLineFrequency =
                PU_POWER_LINE_FREQUENCY_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->PowerLine.bPowerLineFrequency =
                PU_POWER_LINE_FREQUENCY_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->PowerLine.bPowerLineFrequency =
                PU_POWER_LINE_FREQUENCY_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->PowerLine.bPowerLineFrequency =
                PU_POWER_LINE_FREQUENCY_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->PowerLine.bPowerLineFrequency =
                PU_POWER_LINE_FREQUENCY_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->PowerLine.bPowerLineFrequency =
                pVideoDemoCtx->ProcUnitCur.bPowerLineFrequency;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accept all setting from host
            pVideoDemoCtx->ProcUnitCur.bPowerLineFrequency =
                pRequest->PowerLine.bPowerLineFrequency;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_PU_HUE_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->Hue.wHue = PU_HUE_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->Hue.wHue = PU_HUE_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->Hue.wHue = PU_HUE_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->Hue.wHue = PU_HUE_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->Hue.wHue = PU_HUE_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->Hue.wHue = pVideoDemoCtx->ProcUnitCur.wHue;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accpet all setting from host
            pVideoDemoCtx->ProcUnitCur.wHue = pRequest->Hue.wHue;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_PU_SATURATION_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->Saturation.wSaturation = PU_SATURATION_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->Saturation.wSaturation = PU_SATURATION_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->Saturation.wSaturation = PU_SATURATION_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->Saturation.wSaturation = PU_SATURATION_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->Saturation.wSaturation = PU_SATURATION_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->Saturation.wSaturation =
                pVideoDemoCtx->ProcUnitCur.wSaturation;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accpet all setting from host
            pVideoDemoCtx->ProcUnitCur.wSaturation =
                pRequest->Saturation.wSaturation;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_PU_SHARPNESS_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->Sharpness.wSharpness = PU_SHARPNESS_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->Sharpness.wSharpness = PU_SHARPNESS_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->Sharpness.wSharpness = PU_SHARPNESS_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->Sharpness.wSharpness = PU_SHARPNESS_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->Sharpness.wSharpness = PU_SHARPNESS_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->Sharpness.wSharpness =
                pVideoDemoCtx->ProcUnitCur.wSharpness;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accept all setting from host
            pVideoDemoCtx->ProcUnitCur.wSharpness =
                pRequest->Sharpness.wSharpness;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_PU_GAMMA_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->Gamma.wGamma = PU_GAMMA_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->Gamma.wGamma = PU_GAMMA_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->Gamma.wGamma = PU_GAMMA_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->Gamma.wGamma = PU_GAMMA_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->Gamma.wGamma = PU_GAMMA_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->Gamma.wGamma = pVideoDemoCtx->ProcUnitCur.wGamma;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accept all setting from host
            pVideoDemoCtx->ProcUnitCur.wGamma = pRequest->Gamma.wGamma;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->WhiteTemp.wWhiteBalanceTemperature =
                PU_WHITE_BALANCE_TEMPERATURE_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->WhiteTemp.wWhiteBalanceTemperature =
                PU_WHITE_BALANCE_TEMPERATURE_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->WhiteTemp.wWhiteBalanceTemperature =
                PU_WHITE_BALANCE_TEMPERATURE_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->WhiteTemp.wWhiteBalanceTemperature =
                PU_WHITE_BALANCE_TEMPERATURE_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->WhiteTemp.wWhiteBalanceTemperature =
                PU_WHITE_BALANCE_TEMPERATURE_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->WhiteTemp.wWhiteBalanceTemperature =
                pVideoDemoCtx->ProcUnitCur.wWhiteBalanceTemperature;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accpet all setting from host
            pVideoDemoCtx->ProcUnitCur.wWhiteBalanceTemperature =
                pRequest->WhiteTemp.wWhiteBalanceTemperature;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_PU_HUE_AUTO_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->HueAuto.bHueAuto =
                PU_HUE_AUTO_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->HueAuto.bHueAuto =
                PU_HUE_AUTO_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->HueAuto.bHueAuto =
                PU_HUE_AUTO_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->HueAuto.bHueAuto =
                PU_HUE_AUTO_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->HueAuto.bHueAuto =
                PU_HUE_AUTO_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->HueAuto.bHueAuto =
                pVideoDemoCtx->ProcUnitCur.bHueAuto;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accept all setting from host
            pVideoDemoCtx->ProcUnitCur.bHueAuto =
                pRequest->HueAuto.bHueAuto;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_INFO:
            pRequest->WhiteTempAuto.bWhiteBalanceTemperatureAuto =
                PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL_INFO;
            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->WhiteTempAuto.bWhiteBalanceTemperatureAuto =
                PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL_MIN;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->WhiteTempAuto.bWhiteBalanceTemperatureAuto =
                PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL_MAX;
            break;

        case USB_bRequest_Video_GET_RES:
            pRequest->WhiteTempAuto.bWhiteBalanceTemperatureAuto =
                PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL_RES;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->WhiteTempAuto.bWhiteBalanceTemperatureAuto =
                PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL_DEF;
            break;

        case USB_bRequest_Video_GET_CUR:
            pRequest->WhiteTempAuto.bWhiteBalanceTemperatureAuto =
                pVideoDemoCtx->ProcUnitCur.bWhiteBalanceTemperatureAuto;
            break;

        case USB_bRequest_Video_SET_CUR:
            // temporarily just accept all setting from host
            pVideoDemoCtx->ProcUnitCur.bWhiteBalanceTemperatureAuto =
                pRequest->WhiteTempAuto.bWhiteBalanceTemperatureAuto;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;
    }

    // Finally, send control request reply
    VideoClientLib_ControlRequestReply(
        pVideoDemoCtx->pVideo,
        VideoDemoI_ControlRequest_Done,
        pVideoDemoCtx,
        pRequest);
}

static void VideoDemoI_ControlRequest_Done(void* pClientContext, UINT32 ErrorCode) {
    USBPUMP_UNREFERENCED_PARAMETER(pClientContext);
    USBPUMP_UNREFERENCED_PARAMETER(ErrorCode);
}

// Name:    VideoDemo_StreamStatusEvent()
//
// Function:
//     Outswitch function of protocol layer for the streaming interface.
//
// Definition:
//     VOID VideoDemo_StreamStatusEvent(
//        void *pClientContext,
//        UPROTO_VIDEO_STREAM_HANDLE    hVideoStream,
//        BOOL fActivate
//        );
//
// Description:
//     The protocol layer will call this outswitch function to inform application
//     layer when status changes, for example attach, detach.
//
// Returns:
//     None

void VideoDemo_StreamStatusEvent(
    void* pClientContext,
    UPROTO_VIDEO_STREAM_HANDLE    hVideoStream,
    BOOL fActivate
) {
    VIDEODEMO_CONTEXT* CONST    pVideoDemoCtx = pClientContext;
    UPROTO_VIDEO* CONST        pVideo = pVideoDemoCtx->pVideo;

    USBPUMP_TRACE_PARAMETER(pVideo);

    if (hVideoStream == pVideoDemoCtx->hVideoIn) {
        pVideoDemoCtx->fInputActivate = fActivate;

        TTUSB_OBJPRINTF((&pVideo->ObjectHeader, UDMASK_PROTO,
                         " VideoDemo_StreamStatusEvent: VideoIn %s\n",
                         fActivate ? "ACTIVATE" : "DEACTIVATE"));
    }

    if (hVideoStream == pVideoDemoCtx->hVideoOut) {
        pVideoDemoCtx->fOutputActivate = fActivate;

        TTUSB_OBJPRINTF((&pVideo->ObjectHeader, UDMASK_PROTO,
                         " VideoDemo_StreamStatusEvent: VideoOut %s\n",
                         fActivate ? "ACTIVATE" : "DEACTIVATE"));

        // This will trigger the first data to be sent to the host.
        if (fActivate) {
            VideoDemo_Start(pVideoDemoCtx);
        }
    }
}


// Name:    VideoDemo_StreamInterfaceEvent()
//
// Function:
//     Outswitch function of protocol layer to inform the streaming interface of
//     related video request.
//
// Definition:
//    VOID VideoDemo_StreamInterfaceEvent(
//        VOID *                pClientContext,
//        UPROTO_VIDEO_STREAM_HANDLE    hVideoStream,
//        UPROTO_VIDEO_STREAM_REQUEST *    pRequest
//        );
//
// Description:
//    The protocol layer will call this outswitch function when host issues any
//    request to the streaming interface.
//
// Returns:
//     Nothing

void VideoDemo_StreamInterfaceEvent(
    void*  pClientContext,
    UPROTO_VIDEO_STREAM_HANDLE     hVideoStream,
    UPROTO_VIDEO_STREAM_REQUEST*   pRequest) {
    VIDEODEMO_CONTEXT* CONST pVideoDemoCtx = pClientContext;

    pVideoDemoCtx->hStreamRequest = hVideoStream;
    UHIL_cpybuf(&pVideoDemoCtx->StreamRequest, pRequest, sizeof(*pRequest));

    USBPUMP_CALLBACKCOMPLETION_INIT(
        &pVideoDemoCtx->SetupCompletion,
        VideoDemoI_StreamInterfaceEvent_Sync,
        NULL);

    UsbPumpPlatform_PostIfNotBusy(
        pVideoDemoCtx->pPlatform,
        &pVideoDemoCtx->SetupCompletion,
        pVideoDemoCtx);
}

static void VideoDemoI_StreamInterfaceEvent_Sync(void* pClientContext) {
    VIDEODEMO_CONTEXT* CONST pVideoDemoCtx = pClientContext;
    UPROTO_VIDEO_STREAM_REQUEST* pRequest = &pVideoDemoCtx->StreamRequest;

    UsbPumpPlatform_MarkCompletionNotBusy(pVideoDemoCtx->pPlatform,
                                          &pVideoDemoCtx->SetupCompletion);

    pRequest->Hdr.bAcceptRequest = TRUE;

    switch (pRequest->Hdr.bControlSelector) {
    case USBVIDEO_bControlSelector_VS_PROBE_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_CUR:
            pRequest->Video.bmHint = pVideoDemoCtx->ProbeCur.bmHint;
            pRequest->Video.bFormatIndex = pVideoDemoCtx->ProbeCur.bFormatIndex;
            pRequest->Video.bFrameIndex = pVideoDemoCtx->ProbeCur.bFrameIndex;
            pRequest->Video.dwFrameInterval =
                pVideoDemoCtx->ProbeCur.dwFrameInterval;
            pRequest->Video.wKeyFrameRate =
                pVideoDemoCtx->ProbeCur.wKeyFrameRate;
            pRequest->Video.wPFrameRate = pVideoDemoCtx->ProbeCur.wPFrameRate;
            pRequest->Video.wCompQuality = pVideoDemoCtx->ProbeCur.wCompQuality;
            pRequest->Video.wCompWindowSize =
                pVideoDemoCtx->ProbeCur.wCompWindowSize;
            pRequest->Video.wDelay = pVideoDemoCtx->ProbeCur.wDelay;
            pRequest->Video.dwMaxVideoFrameSize =
                pVideoDemoCtx->ProbeCur.dwMaxVideoFrameSize;
            pRequest->Video.dwMaxPayloadTransferSize =
                pVideoDemoCtx->ProbeCur.dwMaxPayloadTransferSize;
            pRequest->Video.dwClockFrequency =
                pVideoDemoCtx->ProbeCur.dwClockFrequency;
            pRequest->Video.bmFramingInfo =
                pVideoDemoCtx->ProbeCur.bmFramingInfo;
            pRequest->Video.bPreferedVersion =
                pVideoDemoCtx->ProbeCur.bPreferedVersion;
            pRequest->Video.bMinVersion = pVideoDemoCtx->ProbeCur.bMinVersion;
            pRequest->Video.bMaxVersion = pVideoDemoCtx->ProbeCur.bMaxVersion;
            break;

        case USB_bRequest_Video_SET_CUR:

            // temporarily just accpet all setting from host, but it's necessary to
            // check each field to see if we want to support it or not
            if (pRequest->Video.bmHint != 0) {
                pVideoDemoCtx->ProbeCur.bmHint = pRequest->Video.bmHint;
            }

            if (pRequest->Video.bFormatIndex != 0)
                pVideoDemoCtx->ProbeCur.bFormatIndex =
                    pRequest->Video.bFormatIndex;

            if (pRequest->Video.bFrameIndex != 0)
                pVideoDemoCtx->ProbeCur.bFrameIndex =
                    pRequest->Video.bFrameIndex;

            if (pRequest->Video.dwFrameInterval != 0)
                pVideoDemoCtx->ProbeCur.dwFrameInterval =
                    pRequest->Video.dwFrameInterval;

            if (pRequest->Video.wKeyFrameRate != 0)
                pVideoDemoCtx->ProbeCur.wKeyFrameRate =
                    pRequest->Video.wKeyFrameRate;

            if (pRequest->Video.wPFrameRate != 0)
                pVideoDemoCtx->ProbeCur.wPFrameRate =
                    pRequest->Video.wPFrameRate;

            if (pRequest->Video.wCompQuality != 0)
                pVideoDemoCtx->ProbeCur.wCompQuality =
                    pRequest->Video.wCompQuality;

            if (pRequest->Video.wCompWindowSize != 0)
                pVideoDemoCtx->ProbeCur.wCompWindowSize =
                    pRequest->Video.wCompWindowSize;

            if (pRequest->Video.wDelay != 0) {
                pVideoDemoCtx->ProbeCur.wDelay = pRequest->Video.wDelay;
            }

            if (pRequest->Video.dwMaxVideoFrameSize != 0)
                pVideoDemoCtx->ProbeCur.dwMaxVideoFrameSize =
                    pRequest->Video.dwMaxVideoFrameSize;

            if (pRequest->Video.dwMaxPayloadTransferSize != 0)
                pVideoDemoCtx->ProbeCur.dwMaxPayloadTransferSize =
                    pRequest->Video.dwMaxPayloadTransferSize;

            if (pRequest->Video.dwClockFrequency != 0)
                pVideoDemoCtx->ProbeCur.dwClockFrequency =
                    pRequest->Video.dwClockFrequency;

            if (pRequest->Video.bmFramingInfo != 0)
                pVideoDemoCtx->ProbeCur.bmFramingInfo =
                    pRequest->Video.bmFramingInfo;

            if (pRequest->Video.bPreferedVersion != 0)
                pVideoDemoCtx->ProbeCur.bPreferedVersion =
                    pRequest->Video.bPreferedVersion;

            if (pRequest->Video.bMinVersion != 0)
                pVideoDemoCtx->ProbeCur.bMinVersion =
                    pRequest->Video.bMinVersion;

            if (pRequest->Video.bMaxVersion != 0)
                pVideoDemoCtx->ProbeCur.bMaxVersion =
                    pRequest->Video.bMaxVersion;

            break;

        case USB_bRequest_Video_GET_MIN:
            pRequest->Video.bmHint = pVideoDemoCtx->ProbeMin.bmHint;
            pRequest->Video.bFormatIndex = pVideoDemoCtx->ProbeMin.bFormatIndex;
            pRequest->Video.bFrameIndex = pVideoDemoCtx->ProbeMin.bFrameIndex;
            pRequest->Video.dwFrameInterval =
                pVideoDemoCtx->ProbeMin.dwFrameInterval;
            pRequest->Video.wKeyFrameRate =
                pVideoDemoCtx->ProbeMin.wKeyFrameRate;
            pRequest->Video.wPFrameRate = pVideoDemoCtx->ProbeMin.wPFrameRate;
            pRequest->Video.wCompQuality = pVideoDemoCtx->ProbeMin.wCompQuality;
            pRequest->Video.wCompWindowSize =
                pVideoDemoCtx->ProbeMin.wCompWindowSize;
            pRequest->Video.wDelay = pVideoDemoCtx->ProbeMin.wDelay;
            pRequest->Video.dwMaxVideoFrameSize =
                pVideoDemoCtx->ProbeMin.dwMaxVideoFrameSize;
            pRequest->Video.dwMaxPayloadTransferSize =
                pVideoDemoCtx->ProbeMin.dwMaxPayloadTransferSize;
            pRequest->Video.dwClockFrequency =
                pVideoDemoCtx->ProbeMin.dwClockFrequency;
            pRequest->Video.bmFramingInfo =
                pVideoDemoCtx->ProbeMin.bmFramingInfo;
            pRequest->Video.bPreferedVersion =
                pVideoDemoCtx->ProbeMin.bPreferedVersion;
            pRequest->Video.bMinVersion = pVideoDemoCtx->ProbeMin.bMinVersion;
            pRequest->Video.bMaxVersion = pVideoDemoCtx->ProbeMin.bMaxVersion;
            break;

        case USB_bRequest_Video_GET_MAX:
            pRequest->Video.bmHint = pVideoDemoCtx->ProbeMax.bmHint;
            pRequest->Video.bFormatIndex = pVideoDemoCtx->ProbeMax.bFormatIndex;
            pRequest->Video.bFrameIndex = pVideoDemoCtx->ProbeMax.bFrameIndex;
            pRequest->Video.dwFrameInterval =
                pVideoDemoCtx->ProbeMax.dwFrameInterval;
            pRequest->Video.wKeyFrameRate =
                pVideoDemoCtx->ProbeMax.wKeyFrameRate;
            pRequest->Video.wPFrameRate = pVideoDemoCtx->ProbeMax.wPFrameRate;
            pRequest->Video.wCompQuality = pVideoDemoCtx->ProbeMax.wCompQuality;
            pRequest->Video.wCompWindowSize =
                pVideoDemoCtx->ProbeMax.wCompWindowSize;
            pRequest->Video.wDelay = pVideoDemoCtx->ProbeMax.wDelay;
            pRequest->Video.dwMaxVideoFrameSize =
                pVideoDemoCtx->ProbeMax.dwMaxVideoFrameSize;
            pRequest->Video.dwMaxPayloadTransferSize =
                pVideoDemoCtx->ProbeMax.dwMaxPayloadTransferSize;
            pRequest->Video.dwClockFrequency =
                pVideoDemoCtx->ProbeMax.dwClockFrequency;
            pRequest->Video.bmFramingInfo =
                pVideoDemoCtx->ProbeMax.bmFramingInfo;
            pRequest->Video.bPreferedVersion =
                pVideoDemoCtx->ProbeMax.bPreferedVersion;
            pRequest->Video.bMinVersion = pVideoDemoCtx->ProbeMax.bMinVersion;
            pRequest->Video.bMaxVersion = pVideoDemoCtx->ProbeMax.bMaxVersion;
            break;

        case USB_bRequest_Video_GET_DEF:
            pRequest->Video.bmHint = pVideoDemoCtx->ProbeDef.bmHint;
            pRequest->Video.bFormatIndex = pVideoDemoCtx->ProbeDef.bFormatIndex;
            pRequest->Video.bFrameIndex = pVideoDemoCtx->ProbeDef.bFrameIndex;
            pRequest->Video.dwFrameInterval =
                pVideoDemoCtx->ProbeDef.dwFrameInterval;
            pRequest->Video.wKeyFrameRate =
                pVideoDemoCtx->ProbeDef.wKeyFrameRate;
            pRequest->Video.wPFrameRate = pVideoDemoCtx->ProbeDef.wPFrameRate;
            pRequest->Video.wCompQuality = pVideoDemoCtx->ProbeDef.wCompQuality;
            pRequest->Video.wCompWindowSize =
                pVideoDemoCtx->ProbeDef.wCompWindowSize;
            pRequest->Video.wDelay = pVideoDemoCtx->ProbeDef.wDelay;
            pRequest->Video.dwMaxVideoFrameSize =
                pVideoDemoCtx->ProbeDef.dwMaxVideoFrameSize;
            pRequest->Video.dwMaxPayloadTransferSize =
                pVideoDemoCtx->ProbeDef.dwMaxPayloadTransferSize;
            pRequest->Video.dwClockFrequency =
                pVideoDemoCtx->ProbeDef.dwClockFrequency;
            pRequest->Video.bmFramingInfo =
                pVideoDemoCtx->ProbeDef.bmFramingInfo;
            pRequest->Video.bPreferedVersion =
                pVideoDemoCtx->ProbeDef.bPreferedVersion;
            pRequest->Video.bMinVersion = pVideoDemoCtx->ProbeDef.bMinVersion;
            pRequest->Video.bMaxVersion = pVideoDemoCtx->ProbeDef.bMaxVersion;
            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_VS_COMMIT_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_CUR:
            pRequest->Video.bmHint = pVideoDemoCtx->CommitCur.bmHint;
            pRequest->Video.bFormatIndex =
                pVideoDemoCtx->CommitCur.bFormatIndex;
            pRequest->Video.bFrameIndex = pVideoDemoCtx->CommitCur.bFrameIndex;
            pRequest->Video.dwFrameInterval =
                pVideoDemoCtx->CommitCur.dwFrameInterval;
            pRequest->Video.wKeyFrameRate =
                pVideoDemoCtx->CommitCur.wKeyFrameRate;
            pRequest->Video.wPFrameRate = pVideoDemoCtx->CommitCur.wPFrameRate;
            pRequest->Video.wCompQuality =
                pVideoDemoCtx->CommitCur.wCompQuality;
            pRequest->Video.wCompWindowSize =
                pVideoDemoCtx->CommitCur.wCompWindowSize;
            pRequest->Video.wDelay = pVideoDemoCtx->CommitCur.wDelay;
            pRequest->Video.dwMaxVideoFrameSize =
                pVideoDemoCtx->CommitCur.dwMaxVideoFrameSize;
            pRequest->Video.dwMaxPayloadTransferSize =
                pVideoDemoCtx->CommitCur.dwMaxPayloadTransferSize;
            pRequest->Video.dwClockFrequency =
                pVideoDemoCtx->CommitCur.dwClockFrequency;
            pRequest->Video.bmFramingInfo =
                pVideoDemoCtx->CommitCur.bmFramingInfo;
            pRequest->Video.bPreferedVersion =
                pVideoDemoCtx->CommitCur.bPreferedVersion;
            pRequest->Video.bMinVersion = pVideoDemoCtx->CommitCur.bMinVersion;
            pRequest->Video.bMaxVersion = pVideoDemoCtx->CommitCur.bMaxVersion;
            break;

        case USB_bRequest_Video_SET_CUR:

            // temporarily just accept all setting from host, but it's necessary to
            // check each field to see if we want to support it or not
            if (pRequest->Video.bmHint != 0) {
                pVideoDemoCtx->CommitCur.bmHint = pRequest->Video.bmHint;
            }

            if (pRequest->Video.bFormatIndex != 0)
                pVideoDemoCtx->CommitCur.bFormatIndex =
                    pRequest->Video.bFormatIndex;

            if (pRequest->Video.bFrameIndex != 0)
                pVideoDemoCtx->CommitCur.bFrameIndex =
                    pRequest->Video.bFrameIndex;

            if (pRequest->Video.dwFrameInterval != 0)
                pVideoDemoCtx->CommitCur.dwFrameInterval =
                    pRequest->Video.dwFrameInterval;

            if (pRequest->Video.wKeyFrameRate != 0)
                pVideoDemoCtx->CommitCur.wKeyFrameRate =
                    pRequest->Video.wKeyFrameRate;

            if (pRequest->Video.wPFrameRate != 0)
                pVideoDemoCtx->CommitCur.wPFrameRate =
                    pRequest->Video.wPFrameRate;

            if (pRequest->Video.wCompQuality != 0)
                pVideoDemoCtx->CommitCur.wCompQuality =
                    pRequest->Video.wCompQuality;

            if (pRequest->Video.wCompWindowSize != 0)
                pVideoDemoCtx->CommitCur.wCompWindowSize =
                    pRequest->Video.wCompWindowSize;

            if (pRequest->Video.wDelay != 0) {
                pVideoDemoCtx->CommitCur.wDelay = pRequest->Video.wDelay;
            }

            if (pRequest->Video.dwMaxVideoFrameSize != 0)
                pVideoDemoCtx->CommitCur.dwMaxVideoFrameSize =
                    pRequest->Video.dwMaxVideoFrameSize;

            if (pRequest->Video.dwMaxPayloadTransferSize != 0)
                pVideoDemoCtx->CommitCur.dwMaxPayloadTransferSize =
                    pRequest->Video.dwMaxPayloadTransferSize;

            if (pRequest->Video.dwClockFrequency != 0)
                pVideoDemoCtx->CommitCur.dwClockFrequency =
                    pRequest->Video.dwClockFrequency;

            if (pRequest->Video.bmFramingInfo != 0)
                pVideoDemoCtx->CommitCur.bmFramingInfo =
                    pRequest->Video.bmFramingInfo;

            if (pRequest->Video.bPreferedVersion != 0)
                pVideoDemoCtx->CommitCur.bPreferedVersion =
                    pRequest->Video.bPreferedVersion;

            if (pRequest->Video.bMinVersion != 0)
                pVideoDemoCtx->CommitCur.bMinVersion =
                    pRequest->Video.bMinVersion;

            if (pRequest->Video.bMaxVersion != 0)
                pVideoDemoCtx->CommitCur.bMaxVersion =
                    pRequest->Video.bMaxVersion;

            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_VS_STILL_PROBE_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_CUR:
            pRequest->Still.bFormatIndex =
                pVideoDemoCtx->StillProbeCur.bFormatIndex;
            pRequest->Still.bFrameIndex =
                pVideoDemoCtx->StillProbeCur.bFrameIndex;
            pRequest->Still.bCompressionIndex =
                pVideoDemoCtx->StillProbeCur.bCompressionIndex;
            pRequest->Still.dwMaxVideoFrameSize =
                pVideoDemoCtx->StillProbeCur.dwMaxVideoFrameSize;
            pRequest->Still.dwMaxPayloadTransferSize =
                pVideoDemoCtx->StillProbeCur.dwMaxPayloadTransferSize;
            break;

        case USB_bRequest_Video_SET_CUR:

            // temporarily just accpet all setting from host, but it's necessary to
            // check each field to see if we want to support it or not
            if (pRequest->Still.bFormatIndex != 0)
                pVideoDemoCtx->StillProbeCur.bFormatIndex =
                    pRequest->Still.bFormatIndex;

            if (pRequest->Still.bFrameIndex != 0)
                pVideoDemoCtx->StillProbeCur.bFrameIndex =
                    pRequest->Still.bFrameIndex;

            if (pRequest->Still.bCompressionIndex != 0)
                pVideoDemoCtx->StillProbeCur.bCompressionIndex =
                    pRequest->Still.bCompressionIndex;

            if (pRequest->Still.dwMaxVideoFrameSize != 0)
                pVideoDemoCtx->StillProbeCur.dwMaxVideoFrameSize =
                    pRequest->Still.dwMaxVideoFrameSize;

            if (pRequest->Still.dwMaxPayloadTransferSize != 0)
                pVideoDemoCtx->StillProbeCur.dwMaxPayloadTransferSize =
                    pRequest->Still.dwMaxPayloadTransferSize;

            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_VS_STILL_COMMIT_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_CUR:
            pRequest->Still.bFormatIndex =
                pVideoDemoCtx->StillCommitCur.bFormatIndex;
            pRequest->Still.bFrameIndex =
                pVideoDemoCtx->StillCommitCur.bFrameIndex;
            pRequest->Still.bCompressionIndex =
                pVideoDemoCtx->StillCommitCur.bCompressionIndex;
            pRequest->Still.dwMaxVideoFrameSize =
                pVideoDemoCtx->StillCommitCur.dwMaxVideoFrameSize;
            pRequest->Still.dwMaxPayloadTransferSize =
                pVideoDemoCtx->StillCommitCur.dwMaxPayloadTransferSize;
            break;

        case USB_bRequest_Video_SET_CUR:

            // temporarily just accpet all setting from host, but it's necessary to
            // check each field to see if we want to support it or not
            if (pRequest->Still.bFormatIndex != 0)
                pVideoDemoCtx->StillCommitCur.bFormatIndex =
                    pRequest->Still.bFormatIndex;

            if (pRequest->Still.bFrameIndex != 0)
                pVideoDemoCtx->StillCommitCur.bFrameIndex =
                    pRequest->Still.bFrameIndex;

            if (pRequest->Still.bCompressionIndex != 0)
                pVideoDemoCtx->StillCommitCur.bCompressionIndex =
                    pRequest->Still.bCompressionIndex;

            if (pRequest->Still.dwMaxVideoFrameSize != 0)
                pVideoDemoCtx->StillCommitCur.dwMaxVideoFrameSize =
                    pRequest->Still.dwMaxVideoFrameSize;

            if (pRequest->Still.dwMaxPayloadTransferSize != 0)
                pVideoDemoCtx->StillCommitCur.dwMaxPayloadTransferSize =
                    pRequest->Still.dwMaxPayloadTransferSize;

            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    case USBVIDEO_bControlSelector_VS_STILL_IMAGE_TRIGGER_CONTROL:
        switch (pRequest->Hdr.bRequest) {
        case USB_bRequest_Video_GET_CUR:
            pRequest->StillImage.bTrigger = pVideoDemoCtx->bStillTrigger;
            break;

        case USB_bRequest_Video_SET_CUR:

            // Still Image Capture
            if (pRequest->StillImage.bTrigger != 0) {
                pVideoDemoCtx->bStillTrigger = pRequest->StillImage.bTrigger;
            }

            break;

        default:
            pRequest->Hdr.bAcceptRequest = FALSE;
            break;
        }

        break;

    default:
        pRequest->Hdr.bAcceptRequest = FALSE;
        break;
    }

    VideoClientLib_StreamRequestReply(pVideoDemoCtx->pVideo,
                                      VideoDemoI_StreamRequest_Done, pVideoDemoCtx, pRequest);
}

static void VideoDemoI_StreamRequest_Done(void* pClientContext, UINT32    ErrorCode) {
    USBPUMP_UNREFERENCED_PARAMETER(pClientContext);
    USBPUMP_UNREFERENCED_PARAMETER(ErrorCode);
}
