/* vscapp_outcall.c Fri Sep 05 2014 16:30:24 chwon */

/*

Module:  vscapp_outcall.c

Function:
Home for gk_UsbPumpProtoVsc2_OutCall

Version:
V3.13b  Fri Sep 05 2014 16:30:24 chwon  Edit level 2

Copyright notice:
This file copyright (C) 2013-2014 by

MCCI Corporation
3520 Krums Corners Road
Ithaca, NY  14850

An unpublished work.  All rights reserved.

This file is proprietary information, and may not be disclosed or
copied without the prior permission of MCCI Corporation

Author:
ChaeHee Won, MCCI Corporation December 2013

Revision history:
3.11d  Wed Dec 18 2013 11:16:34  chwon
17949: Module created.

3.13b  Fri Sep 05 2014 16:30:24  chwon
18506: Fix diab compiler warning -- make static function.

*/

#include "usbpump_vsc2app.h"
#include "usbpump_proto_vsc2_api.h"
#include "usbpumpapi.h"
#include "usbpumpdebug.h"
#include "usbpumplib.h"
#include "uplatformapi.h"
#include "vsc2app_outcall.h"
#include "uendpoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <mv_types.h>
/****************************************************************************\
  |
  |   Manifest constants & typedefs.
  |
  | This is strictly for private types and constants which will not
  | be exported.
  |
  \****************************************************************************/

static
USBPUMP_PROTO_VSC2_EVENT_FN
UsbPumpVscAppI_Event;

static
USBPUMP_PROTO_VSC2_SETUP_VALIDATE_FN
UsbPumpVscAppI_SetupValidate;

static
USBPUMP_PROTO_VSC2_SETUP_PROCESS_FN
UsbPumpVscAppI_SetupProcess;

static
VOID
UsbPumpVscAppI_StartRead(
    USBPUMP_VSC2APP_CONTEXT * pSelf,
    __TMS_UINT32 endPoint
    );

static
VOID
UsbPumpVscAppI_StartWrite(
    USBPUMP_VSC2APP_CONTEXT * pSelf,
    __TMS_UINT32 endPoint
    );

static
UBUFIODONEFN
UsbPumpVscAppI_ReadTransferDone;

static
UBUFIODONEFN
UsbPumpVscAppI_WriteTransferDone;

VOID
UsbVscAppRead(
    USBPUMP_VSC2APP_CONTEXT * pSelf,
    __TMS_UINT32  size,
    __TMS_CHAR *  buff,
    __TMS_UINT32 endPoint
    );

VOID
UsbVscAppWrite(
    USBPUMP_VSC2APP_CONTEXT * pSelf,
    __TMS_UINT32  size,
    __TMS_CHAR *  buff,
    __TMS_UINT32 endPoint
    );

static UCALLBACKFN
UsbVscAppRead_Callback;

static UCALLBACKFN
UsbVscAppWrite_Callback;

typedef struct {
  USBPUMP_VSC2APP_CONTEXT *pVscContext;
  UCALLBACKCOMPLETION Callback;
  UINT8 epNo;
} USBVSC_RDWR_CONTEXT;

/****************************************************************************\
  |
  | Read-only data.
  |
  | If program is to be ROM-able, these must all be tagged read-only
  | using the ROM storage class; they may be global.
  |
  \****************************************************************************/

CONST USBPUMP_PROTO_VSC2_OUTCALL  gk_UsbPumpProtoVsc2_OutCall =
USBPUMP_PROTO_VSC2_OUTCALL_INIT_V1(
    UsbPumpVscAppI_Event,
    UsbPumpVscAppI_SetupValidate,
    UsbPumpVscAppI_SetupProcess
    );


/****************************************************************************\
  |
  | VARIABLES:
  |
  | If program is to be ROM-able, these must be initialized
  | using the BSS keyword.  (This allows for compilers that require
  | every variable to have an initializer.)  Note that only those
  | variables owned by this module should be declared here, using the BSS
  | keyword; this allows for linkers that dislike multiple declarations
  | of objects.
  |
  \****************************************************************************/



/*

   Name: UsbPumpVscAppI_Event

   Function:
   Deliver vsc protocol event to the registered client

   Definition:
   VOID
   UsbPumpVscAppI_Event(
   VOID *        ClientHandle,
   USBPUMP_PROTO_VSC2_EVENT  Event,
   CONST VOID *      pEventInfo
   );

   Description:
   This is vsc protocol driver out-call function.  It will be called
   by vsc protocol driver to deliver vsc protocol event to the
   registered client.

   Returns:
   No explicit result.

*/

static
  VOID
UsbPumpVscAppI_Event(
    VOID *        ClientHandle,
    USBPUMP_PROTO_VSC2_EVENT  Event,
    CONST VOID *      pEventInfo
    )
{
  USBPUMP_VSC2APP_CONTEXT * CONST pSelf = ClientHandle;

  USBPUMP_UNREFERENCED_PARAMETER(pEventInfo);

  switch (Event)
  {
    case USBPUMP_PROTO_VSC2_EVENT_INTERFACE_UP:
      {
        TTUSB_PLATFORM_PRINTF((
              pSelf->pPlatform,
              UDMASK_ANY,
              " UsbPumpVscAppI_Event: interface up.\n"
              ));
        DUSBPRINT("\nIfc up\n");
        pSelf->fInterfaceUp = TRUE;
        if (pSelf->usbInterfaceUpCB != NULL) {
          pSelf->usbInterfaceUpCB(0);
        }
        // TODO(hyx): The following line is only for xlink, so we comment it here
        // rtems_semaphore_release(pSelf->semReadId[0]); //using the first read semaphore for init done.
      }
      break;

    case USBPUMP_PROTO_VSC2_EVENT_INTERFACE_DOWN:
      {
        TTUSB_PLATFORM_PRINTF((
              pSelf->pPlatform,
              UDMASK_ANY,
              " UsbPumpVscAppI_Event: interface down.\n"
              ));

        pSelf->fInterfaceUp = FALSE;
        if (pSelf->usbInterfaceDownCB != NULL) {
          pSelf->usbInterfaceDownCB(0);
        }
      }
      break;

    case USBPUMP_PROTO_VSC2_EVENT_RESUME:
    case USBPUMP_PROTO_VSC2_EVENT_SUSPEND:
      /* Need to notify client */
      break;

    default:
      break;
  }
}

/*

   Name: UsbPumpVscAppI_SetupValidate

   Function:
   Validate vsc control request to the registered client.

   Definition:
   USBPUMP_PROTO_VSC2_SETUP_STATUS
   UsbPumpVscAppI_SetupValidate(
   VOID *    ClientHandle,
   CONST USETUP *  pSetup
   );

   Description:
   This is vsc protocol driver out-call function.  It will be called by
   vsc protocol driver when receive vendor specific request from host.
   Client provided USBPUMP_PROTO_VSC2_SETUP_VALIDATE_FN() should validate
   vendor specific request.  If client can accept this control request,
   client should return USBPUMP_PROTO_VSC2_SETUP_STATUS_ACCEPTED.  If this
   control request is unknown (can't accept this control request),
   client should return USBPUMP_PROTO_VSC2_SETUP_STATUS_NOT_CLAIMED.
   If client knows this request but wants to reject this request, client
   should return USBPUMP_PROTO_VSC2_SETUP_STATUS_REJECTED.

   If client accepted this vendor specific control request, client will
   get USBPUMP_PROTO_VSC2_SETUP_PROCESS_FN() callback to process accepted
   control request.

   If client returns USBPUMP_PROTO_VSC2_SETUP_STATUS_REJECTED, vsc protocol
   will send STALL.

   If client returns USBPUMP_PROTO_VSC2_SETUP_STATUS_NOT_CLAIMED, protocol
   do nothing for this vendor specific command.

   Returns:
   USBPUMP_PROTO_VSC2_SETUP_STATUS

*/

static
  USBPUMP_PROTO_VSC2_SETUP_STATUS
UsbPumpVscAppI_SetupValidate(
    VOID *    ClientHandle,
    CONST USETUP *  pSetup
    )
{
  USBPUMP_VSC2APP_CONTEXT * CONST pSelf = ClientHandle;
  USBPUMP_PROTO_VSC2_SETUP_STATUS Status;

  pSelf->fAcceptSetup = FALSE;
  Status = USBPUMP_PROTO_VSC2_SETUP_STATUS_NOT_CLAIMED;

  if (pSetup->uc_bmRequestType == USB_bmRequestType_HVDEV)
  {
    if (pSetup->uc_bRequest == 0)
    {
      pSelf->fAcceptSetup = TRUE;
      Status = USBPUMP_PROTO_VSC2_SETUP_STATUS_ACCEPTED;
    }
    else
    {
      Status = USBPUMP_PROTO_VSC2_SETUP_STATUS_REJECTED;
    }
  }
  else if (pSetup->uc_bmRequestType == USB_bmRequestType_DVDEV)
  {
    if (pSetup->uc_bRequest == 0)
    {
      pSelf->fAcceptSetup = TRUE;
      Status = USBPUMP_PROTO_VSC2_SETUP_STATUS_ACCEPTED;
    }
    else
    {
      Status = USBPUMP_PROTO_VSC2_SETUP_STATUS_REJECTED;
    }
  }

  return Status;
}

/*

   Name: UsbPumpVscAppI_SetupProcess

   Function:
   Process vsc control request.

   Definition:
   BOOL
   UsbPumpVscAppI_SetupProcess(
   VOID *    ClientHandle,
   CONST USETUP *  pSetup,
   VOID *    pBuffer,
   UINT16    nBuffer
   );

   Description:
   This is vsc protocol driver out-call function.  It will be called by
   vsc protocol driver to process vendor specific control request.

   If direction of vendor request is from host to device, vsc protocol
   receives data from host and protocol passes received data thru pBuffer
   and nBuffer.  Client processes control data in the pBuffer and client
   should send reply using USBPUMP_PROTO_VSC2_CONTROL_REPLY_FN().

   If direction of vendor request is from device to host, vsc protocol
   provide data buffer (pBuffer and nBuffer).  Client processes control
   request and copies data to the pBuffer. Client should send reply using
   USBPUMP_PROTO_VSC2_CONTROL_REPLY_FN().

   If this process function returns FALSE, the VSC protocol driver will
   send STALL for this setup packet.  If it returns TRUE, client should
   handle this setup packet.

   Returns:
   TRUE if client process this setup packet.  FALSE if client doesn't
   want to process this setup packet and want to send STALL.

*/

static
  BOOL
UsbPumpVscAppI_SetupProcess(
    VOID *    ClientHandle,
    CONST USETUP *  pSetup,
    VOID *    pBuffer,
    UINT16    nBuffer
    )
{
  USBPUMP_VSC2APP_CONTEXT * CONST pSelf = ClientHandle;

  if (! pSelf->fAcceptSetup)
    return FALSE;

  pSelf->fAcceptSetup = FALSE;

  if (pSetup->uc_bmRequestType == USB_bmRequestType_HVDEV)
  {
    /* pSelf->pControlBuffer has received data from host. */
    TTUSB_PLATFORM_PRINTF((
          pSelf->pPlatform,
          UDMASK_ANY,
          " UsbPumpVscApp_Setup: Received %d bytes:"
          " %02x %02x %02x %02x ...\n",
          nBuffer,
          ((UINT8 *) pBuffer)[0],
          ((UINT8 *) pBuffer)[1],
          ((UINT8 *) pBuffer)[2],
          ((UINT8 *) pBuffer)[3]
          ));

    /* Send control status data */
    (*pSelf->InCall.Vsc.pControlReplyFn)(
        pSelf->hSession,
        pBuffer,
        0
        );
  }
  else if (pSetup->uc_bmRequestType == USB_bmRequestType_DVDEV)
  {
    UINT16  Size;

    /* Send control data -- just copy SETUP packet */
    Size = sizeof(*pSetup);
    if (Size > nBuffer)
      Size = nBuffer;

    UHIL_cpybuf(pBuffer, pSetup, Size);

    (*pSelf->InCall.Vsc.pControlReplyFn)(
        pSelf->hSession,
        pBuffer,
        Size
        );
  }
  else
  {
    /* Send reply... here just STALL */
    (*pSelf->InCall.Vsc.pControlReplyFn)(
        pSelf->hSession,
        NULL,
        0
        );
  }

  return TRUE;
}

/*

   Name: UsbPumpVscAppI_StartRead

   Function:
   Start loopback

   Definition:
   VOID
   UsbPumpVscAppI_StartRead(
   USBPUMP_VSC2APP_CONTEXT * pSelf
   );

   Description:
   This function starts data loopback operations.

   Returns:
   No explicit result.

*/

static
  VOID
UsbPumpVscAppI_StartRead(
    USBPUMP_VSC2APP_CONTEXT * pSelf,
    __TMS_UINT32 endPoint
    )
{
  UBUFQE *      pQeOut;
  USBPUMP_VSC2APP_REQUEST * pRequest;
  USBPUMP_PROTO_VSC2_STREAM_HANDLE    hStreamOut;
  UNUSED(pRequest);

  hStreamOut = pSelf->hStreamOut[endPoint];
  if ((pQeOut = UsbGetQe(&pSelf->pFreeQeHeadOut)) == NULL)
  {
    DUSBPRINT("No free UBUFQE\n");
  }
  else
  {
    USBPUMP_VSC2APP_REQUEST * CONST
      pRequest = __TMS_CONTAINER_OF(
          pQeOut,
          USBPUMP_VSC2APP_REQUEST,
          Vsc.Qe.UbufqeLegacy
          );
    UBUFQE_FLAGS  Flags;
    Flags = UBUFQEFLAG_SHORTCOMPLETES;

    UsbPumpProtoVsc2Request_PrepareLegacy(
        &pRequest->Vsc,
        hStreamOut,
        pSelf->rBuff[endPoint],
        NULL, /* hBuffer */
        pSelf->rSize[endPoint],
        Flags,  /* TransferFlags */
        UsbPumpVscAppI_ReadTransferDone,
        pSelf
        );

    /* submit request to send loopback data */
    DUSBPRINT("submit read request on EP %d\n", endPoint+1);
    (*pSelf->InCall.Vsc.pSubmitRequestFn)(
        pSelf->hSession,
        &pRequest->Vsc
        );
  }
}

static
  VOID
UsbPumpVscAppI_StartWrite(
    USBPUMP_VSC2APP_CONTEXT * pSelf,
    __TMS_UINT32 endPoint
    )
{
  UBUFQE *      pQeIn;
  USBPUMP_VSC2APP_REQUEST * pRequestIn;
  __TMS_USBPUMP_PROTO_VSC2_STREAM_HANDLE    hStreamIn;
  UNUSED(pRequestIn);

  hStreamIn = pSelf->hStreamIn[endPoint];

  if ((pQeIn = UsbGetQe(&pSelf->pFreeQeHeadIn)) == NULL)
  {
    DUSBPRINT("No free UBUFQE\n");
  }
  else
  {
    USBPUMP_VSC2APP_REQUEST * CONST
      pRequestIn = __TMS_CONTAINER_OF(
          pQeIn,
          USBPUMP_VSC2APP_REQUEST,
          Vsc.Qe.UbufqeLegacy
          );
    UBUFQE_FLAGS  Flags;
    Flags = pRequestIn->Vsc.Qe.UbufqeLegacy.uqe_flags & UBUFQEFLAG_POSTBREAK;

    UsbPumpProtoVsc2Request_PrepareLegacy(
        &pRequestIn->Vsc,
        hStreamIn,
        pSelf->wBuff[endPoint],
        NULL, /* hBuffer */
        pSelf->wSize[endPoint],
        Flags,  /* TransferFlags */
        UsbPumpVscAppI_WriteTransferDone,
        pSelf
        );

    /* submit request to send loopback data */
    DUSBPRINT("submit write request on EP %d\n", endPoint+1);
    (*pSelf->InCall.Vsc.pSubmitRequestFn)(
        pSelf->hSession,
        &pRequestIn->Vsc
        );
  }
}

static
  VOID
UsbPumpVscAppI_ReadTransferDone(
    UDEVICE * pDevice,
    UENDPOINT * pUep,
    UBUFQE *  pQe
    )
{
  USBPUMP_VSC2APP_REQUEST * CONST
    pRequest = __TMS_CONTAINER_OF(
        pQe,
        USBPUMP_VSC2APP_REQUEST,
        Vsc.Qe.UbufqeLegacy
        );
  USBPUMP_VSC2APP_CONTEXT * CONST pSelf = pQe->uqe_doneinfo;

  __TMS_UINT32 endPoint = pUep->uep_hh.uephh_pPipe->upipe_bEndpointAddress; //endpoint number

  USBPUMP_UNREFERENCED_PARAMETER(pDevice);
  USBPUMP_UNREFERENCED_PARAMETER(pUep);

  if (pQe->uqe_status != USTAT_OK)
  {
    TTUSB_PLATFORM_PRINTF((
          pSelf->pPlatform,
          UDMASK_ERRORS,
          "?UsbPumpVscAppI_ReadTransferDone:"
          pRequest,
          UsbPumpStatus_Name(pQe->uqe_status),
          pQe->uqe_status
          ));
  }

  else
  {
    rtems_semaphore_release(pSelf->semReadId[endPoint-1]);
  }

  UsbPutQe(&pSelf->pFreeQeHeadOut, &pRequest->Vsc.Qe.UbufqeLegacy);
  DUSBPRINT("Read done on EP %d\n", endPoint);
}

static
  VOID
UsbPumpVscAppI_WriteTransferDone(
    UDEVICE * pDevice,
    UENDPOINT * pUep,
    UBUFQE *  pQe
    )
{
  USBPUMP_VSC2APP_REQUEST * CONST
    pRequest = __TMS_CONTAINER_OF(
        pQe,
        USBPUMP_VSC2APP_REQUEST,
        Vsc.Qe.UbufqeLegacy
        );
  USBPUMP_VSC2APP_CONTEXT * CONST pSelf = pQe->uqe_doneinfo;

  __TMS_UINT32 endPoint = pUep->uep_hh.uephh_pPipe->upipe_bEndpointAddress % 0x8; //endpoint number

  USBPUMP_UNREFERENCED_PARAMETER(pDevice);
  USBPUMP_UNREFERENCED_PARAMETER(pUep);

  if (pQe->uqe_status != USTAT_OK)
  {
    TTUSB_PLATFORM_PRINTF((
          pSelf->pPlatform,
          UDMASK_ERRORS,
          "?UsbPumpVscAppI_ReadTransferDone:"
          pRequest,
          UsbPumpStatus_Name(pQe->uqe_status),
          pQe->uqe_status
          ));
  }

  else
  {
    rtems_semaphore_release(pSelf->semWriteId[endPoint-1]);
  }

  UsbPutQe(&pSelf->pFreeQeHeadIn, &pRequest->Vsc.Qe.UbufqeLegacy);
  DUSBPRINT("Write done on EP %d\n", endPoint);
}

// Read/Write API

  VOID
UsbVscAppRead(USBPUMP_VSC2APP_CONTEXT * pSelf, __TMS_UINT32 size, __TMS_CHAR * buff, __TMS_UINT32 endPoint)
{
  USBVSC_RDWR_CONTEXT *pReadContext;

  pReadContext = malloc(sizeof(*pReadContext));
  if (pReadContext) {
    pReadContext->pVscContext = pSelf;
    pReadContext->epNo = endPoint;
  }
  pSelf->rBuff[endPoint] = buff;
  pSelf->rSize[endPoint] = size;

  USBPUMP_CALLBACKCOMPLETION_INIT(
      &pReadContext->Callback,
      UsbVscAppRead_Callback,
      pReadContext
      );

  UsbPumpPlatform_PostEvent(
      pSelf->pPlatform,
      &pReadContext->Callback
      );
}

  VOID
UsbVscAppWrite(USBPUMP_VSC2APP_CONTEXT * pSelf, __TMS_UINT32 size, __TMS_CHAR * buff, __TMS_UINT32 endPoint)
{
  USBVSC_RDWR_CONTEXT *pWriteContext;

  pWriteContext = malloc(sizeof(*pWriteContext));

  if (pWriteContext) {
    pWriteContext->pVscContext = pSelf;
    pWriteContext->epNo = endPoint;
  }
  pSelf->wBuff[endPoint] = buff;
  pSelf->wSize[endPoint] = size;

  USBPUMP_CALLBACKCOMPLETION_INIT(
      &pWriteContext->Callback,
      UsbVscAppWrite_Callback,
      pWriteContext
      );

  UsbPumpPlatform_PostEvent(
      pSelf->pPlatform,
      &pWriteContext->Callback
      );
}

static
  VOID
UsbVscAppRead_Callback( VOID *  pContext)
{
  USBVSC_RDWR_CONTEXT * CONST ctx = pContext;

  UsbPumpVscAppI_StartRead(ctx->pVscContext, ctx->epNo);
  free(ctx);
}

static
  VOID
UsbVscAppWrite_Callback( VOID *  pContext)
{
  USBVSC_RDWR_CONTEXT * CONST ctx = pContext;

  UsbPumpVscAppI_StartWrite(ctx->pVscContext, ctx->epNo);
  free(ctx);
}

