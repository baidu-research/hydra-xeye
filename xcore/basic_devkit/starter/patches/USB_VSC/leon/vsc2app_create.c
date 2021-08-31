/* vscapp_create.c  Thu Sep 25 2014 13:57:34 chwon */

/*

Module:  vscapp_create.c

Function:
  Home for UsbPumpVsc2App_ClientCreate()

Version:
  V3.13b  Thu Sep 25 2014 13:57:34 chwon  Edit level 2

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
   3.11d  Wed Dec 18 2013 11:00:40  chwon
  17949: Module created.

   3.13b  Thu Sep 25 2014 13:57:34  chwon
  17949: Fix debug message format.

*/

#include "usbpump_vsc2app.h"
#include "usbpumpapi.h"
#include "usbpumplib.h"
#include "uplatformapi.h"
#include "usbpumpobjectapi.h"
#include "usbpumproot.h"
#include "usbpumpdebug.h"
#include "upipe.h"

/****************************************************************************\
|
|   Manifest constants & typedefs.
|
| This is strictly for private types and constants which will not
| be exported.
|
\****************************************************************************/
USBPUMP_VSC2APP_CONTEXT* pSelf;


static
VOID
UsbPumpVsc2App_ClientDelete(
);

static
USBPUMP_API_OPEN_CB_FN
UsbPumpVsc2App_OpenSession_Callback;


/****************************************************************************\
|
| Read-only data.
|
| If program is to be ROM-able, these must all be tagged read-only
| using the ROM storage class; they may be global.
|
\****************************************************************************/


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

Name: UsbPumpVsc2App_ClientCreate

Function:
  Generate the leaf functions and bindings for the vsc demo app

Definition:
  USBPUMP_VSC2APP_CONTEXT *
  UsbPumpVsc2App_ClientCreate(
    UPLATFORM * pPlatform
    );

Description:
  This module scans the named functions of the DataPump, looking
  for objects named "vsc.*.fn.mcci.com".  For each of these,
  the module creates device instances and initializes the datastructures.

Returns:
  Pointer of USBPUMP_VSC2APP_CONTEXT if success.

Notes:
  This function should be called in the DataPump context.

*/

USBPUMP_VSC2APP_CONTEXT*
UsbPumpVsc2App_ClientCreate(
    UPLATFORM* pPlatform
) {
    USBPUMP_OBJECT_ROOT*    pRootObject;
    USBPUMP_OBJECT_HEADER*    pObjectHeader;
    BOOL        StatusOk;

    pSelf = UsbPumpPlatform_MallocZero(pPlatform, sizeof(*pSelf));

    if (pSelf == NULL) {
        TTUSB_PLATFORM_PRINTF((
                                  pPlatform,
                                  UDMASK_ERRORS | UDMASK_ANY,
                                  "?UsbPumpVsc2App_ClientCreate:"
                                  " can't allocate VSC application context!\n"
                              ));
        return NULL;
    }

    pSelf->pPlatform = pPlatform;

    /*
    || Find VSC protocol instance object.
    */
    pRootObject = UsbPumpObject_GetRoot(&pPlatform->upf_Header);
    pObjectHeader = UsbPumpObject_EnumerateMatchingNames(
                        &pRootObject->Header,
                        NULL,
                        gk_ProtoVsc2_ObjectName
                    );

    StatusOk = pObjectHeader != NULL;

    if (! StatusOk) {
        TTUSB_PLATFORM_PRINTF((
                                  pPlatform,
                                  UDMASK_ERRORS | UDMASK_ANY,
                                  "?UsbPumpVsc2App_ClientCreate:"
                                  " can't find VSC protocol object\n"
                              ));
    } else {
        pSelf->pProtoVscObject = pObjectHeader;
        pSelf->pDevice = UsbPumpObject_GetDevice(pObjectHeader);

        if (pSelf->pDevice == NULL) {
            TTUSB_PLATFORM_PRINTF((
                                      pPlatform,
                                      UDMASK_ERRORS | UDMASK_ANY,
                                      "?UsbPumpVsc2App_ClientCreate:"
                                      " can't get UDEVICE object.\n"
                                  ));
            StatusOk = FALSE;
        }
    }

    if (StatusOk) {
        USBPUMP_VSC2APP_REQUEST* pRequest;
        UINT        i;

        pRequest = pSelf->Requests;

        for (i = 0; i < USBPUMP_VSC2APP_NUM_REQUEST_OUT; ++i) {
            UsbPutQe(
                &pSelf->pFreeQeHeadOut,
                &pRequest->Vsc.Qe.UbufqeLegacy
            );
            ++pRequest;
        }

        for (i = 0; i < USBPUMP_VSC2APP_NUM_REQUEST_IN; ++i) {
            UsbPutQe(
                &pSelf->pFreeQeHeadIn,
                &pRequest->Vsc.Qe.UbufqeLegacy
            );
            ++pRequest;
        }
    }

    if (StatusOk) {
        CONST RECSIZE SizeOpenMemory =
            UsbPumpObject_SizeOpenSessionRequestMemory(
                pObjectHeader
            );

        UsbPumpObject_OpenSession(
            pObjectHeader,
            UsbPumpPlatform_Malloc(pPlatform, SizeOpenMemory),
            SizeOpenMemory,
            UsbPumpVsc2App_OpenSession_Callback,
            pSelf,  /* pCallBackContext */
            &gk_UsbPumpProtoVsc2_Guid,
            NULL, /* pClientObject -- OPTIONAL */
            &pSelf->InCall.GenericCast,
            sizeof(pSelf->InCall),
            pSelf,  /* pClientHandle */
            &gk_UsbPumpProtoVsc2_OutCall.GenericCast,
            sizeof(gk_UsbPumpProtoVsc2_OutCall)
        );

        TTUSB_PLATFORM_PRINTF((
                                  pPlatform,
                                  UDMASK_ANY,
                                  " UsbPumpVsc2App_ClientCreate:"
                                  " pSelf(%p) created for pProtoVscObject(%p).\n",
                                  pSelf,
                                  pSelf->pProtoVscObject
                              ));

        return pSelf;
    } else {
        UsbPumpVsc2App_ClientDelete(pSelf);
        return NULL;
    }
}

static
VOID
UsbPumpVsc2App_ClientDelete(
) {
    UBUFQE*   pQe;

    while ((pQe = UsbGetQe(&pSelf->pFreeQeHeadOut)) != NULL) {
        USBPUMP_VSC2APP_REQUEST* CONST pRequest =
            __TMS_CONTAINER_OF(
                pQe,
                USBPUMP_VSC2APP_REQUEST,
                Vsc.Qe.UbufqeLegacy
            );

        UsbFreeDeviceBuffer(
            pSelf->pDevice,
            pRequest->pBuffer,
            pRequest->nBuffer
        );
    }

    while ((pQe = UsbGetQe(&pSelf->pFreeQeHeadIn)) != NULL) {
        USBPUMP_VSC2APP_REQUEST* CONST pRequest =
            __TMS_CONTAINER_OF(
                pQe,
                USBPUMP_VSC2APP_REQUEST,
                Vsc.Qe.UbufqeLegacy
            );

        UsbFreeDeviceBuffer(
            pSelf->pDevice,
            pRequest->pBuffer,
            pRequest->nBuffer
        );
    }

    UsbPumpPlatform_Free(pSelf->pPlatform, pSelf, sizeof(*pSelf));
}

static VOID
UsbPumpVsc2App_OpenSession_Callback(
    VOID*       pClientContext,
    USBPUMP_SESSION_HANDLE  SessionHandle,
    UINT32      Result,
    VOID*       pOpenRequestMemory,
    RECSIZE     sizeOpenRequestMemory
) {
    pSelf = pClientContext;
    USBPUMP_PROTO_VSC2_STATUS Status;
    UINT32 i;

    TTUSB_PLATFORM_PRINTF((
                              pSelf->pPlatform,
                              UDMASK_ENTRY | UDMASK_ANY,
                              "+UsbPumpVsc2App_OpenSession_Callback:"
                              " Result=%d, SessionHandle(%#x)\n",
                              Result,
                              SessionHandle
                          ));

    if (pOpenRequestMemory != NULL) {
        UsbPumpPlatform_Free(
            pSelf->pPlatform,
            pOpenRequestMemory,
            sizeOpenRequestMemory
        );
    }

    if (Result != USBPUMP_API_STATUS_OK) {
        TTUSB_PLATFORM_PRINTF((
                                  pSelf->pPlatform,
                                  UDMASK_ERRORS,
                                  "?UsbPumpVsc2App_OpenSession_Callback:"
                                  " failed to open a vsc session (%d)\n",
                                  Result
                              ));
        UsbPumpVsc2App_ClientDelete(pSelf);
        return;
    }

    /* Save SessionHandle */
    pSelf->hSession = SessionHandle;

    /* Open datastreams */
    for (i = 0; i < USBPUMP_VSC2APP_NUM_EP_OUT; i++) {
        Status = (*pSelf->InCall.Vsc.pOpenStreamFn)(
                     pSelf->hSession,
                     UPIPE_SETTING_MASK_BULK_OUT,
                     0,  /* EpAddrMask */
                     i,  /* OrdPipe */
                     &pSelf->hStreamOut[i]
                 );

        if (! USBPUMP_PROTO_VSC2_STATUS_SUCCESS(Status)) {
            TTUSB_PLATFORM_PRINTF((
                                      pSelf->pPlatform,
                                      UDMASK_ERRORS,
                                      "?UsbPumpVsc2App_OpenSession_Callback:"
                                      " failed to open bulk IN stream.\n"
                                  ));
        }
    }

    for (i = 0; i < USBPUMP_VSC2APP_NUM_EP_IN; i++) {
        Status = (*pSelf->InCall.Vsc.pOpenStreamFn)(
                     pSelf->hSession,
                     UPIPE_SETTING_MASK_BULK_IN,
                     0,  /* EpAddrMask */
                     i,  /* OrdPipe */
                     &pSelf->hStreamIn[i]
                 );

        if (! USBPUMP_PROTO_VSC2_STATUS_SUCCESS(Status)) {
            TTUSB_PLATFORM_PRINTF((
                                      pSelf->pPlatform,
                                      UDMASK_ERRORS,
                                      "?UsbPumpVsc2App_OpenSession_Callback:"
                                      " failed to open bulk IN stream %d.\n", i
                                  ));
        }
    }
}

/**** end of vscapp_create.c ****/
