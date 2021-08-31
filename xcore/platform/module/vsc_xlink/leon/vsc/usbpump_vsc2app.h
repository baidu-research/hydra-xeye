/* usbpump_vsc2app.h    Wed Dec 18 2013 11:02:46 chwon */

/*

Module:  usbpump_vsc2app.h

Function:
    Definition of VSC application

Version:
    V3.11d  Wed Dec 18 2013 11:02:46 chwon  Edit level 1

Copyright notice:
    This file copyright (C) 2013 by

        MCCI Corporation
        3520 Krums Corners Road
        Ithaca, NY  14850

    An unpublished work.  All rights reserved.

    This file is proprietary information, and may not be disclosed or
    copied without the prior permission of MCCI Corporation

Author:
    ChaeHee Won, MCCI Corporation   December 2013

Revision history:
   3.11d  Wed Dec 18 2013 11:02:46 chwon
    17949: Module created.

*/
#include <rtems.h>

#ifndef _USBPUMP_VSC2APP_H_ /* prevent multiple includes */
#define _USBPUMP_VSC2APP_H_

#ifndef _USBPUMP_PROTO_VSC2_H_
# include "usbpump_proto_vsc2.h"
#endif

#ifndef _USBPUMP_PROTO_VSC2_CONFIG_H_
# include "usbpump_proto_vsc2_config.h"
#endif

#ifndef _USBPUMP_PROTO_VSC2_API_H_
# include "usbpump_proto_vsc2_api.h"
#endif

#ifndef _USBPUMP_PROTO_VSC2_REQUEST_H_
# include "usbpump_proto_vsc2_request.h"
#endif

#ifndef _USBAPPINIT_H_
# include "usbappinit.h"
#endif

#ifndef _UPLATFORM_H_
# include "uplatform.h"
#endif

#ifndef _UDEVICE_H_
# include "udevice.h"
#endif

#ifndef _USBPUMPOBJECT_H_
# include "usbpumpobject.h"
#endif

/****************************************************************************\
|
|   Constants
|
\****************************************************************************/
#define USBPUMP_VSC2APP_NUM_EP_OUT 3
#define USBPUMP_VSC2APP_NUM_EP_IN 3

#ifndef USBPUMP_VSC2APP_NUM_REQUEST_OUT     /* PARAM */
# define USBPUMP_VSC2APP_NUM_REQUEST_OUT    (16)
#endif

#ifndef USBPUMP_VSC2APP_NUM_REQUEST_IN      /* PARAM */
# define USBPUMP_VSC2APP_NUM_REQUEST_IN     (16)
#endif

#define USBPUMP_VSC2APP_NUM_REQUEST         \
    (USBPUMP_VSC2APP_NUM_REQUEST_OUT + USBPUMP_VSC2APP_NUM_REQUEST_IN)

/****************************************************************************\
|
|   USBPUMP_VSC2APP_CONTEXT structure definition
|
\****************************************************************************/

__TMS_TYPE_DEF_STRUCT(USBPUMP_VSC2APP_CONTEXT);
__TMS_TYPE_DEF_STRUCT(USBPUMP_VSC2APP_REQUEST);

typedef void (*CALLBACK_FN)(int);

struct __TMS_STRUCTNAME(USBPUMP_VSC2APP_REQUEST) {
    __TMS_USBPUMP_PROTO_VSC2_REQUEST    Vsc;
    __TMS_VOID*                 pBuffer;
    __TMS_BYTES             nBuffer;
    __TMS_USBPUMP_PROTO_VSC2_STREAM_HANDLE  hStreamIn;
};

struct __TMS_STRUCTNAME(USBPUMP_VSC2APP_CONTEXT) {
    __TMS_USBPUMP_OBJECT_HEADER*        pProtoVscObject;

    __TMS_UPLATFORM*            pPlatform;
    __TMS_UDEVICE*              pDevice;

    __TMS_USBPUMP_SESSION_HANDLE        hSession;
    __TMS_USBPUMP_PROTO_VSC2_INCALL     InCall;

    __TMS_UINT32                fInterfaceUp: 1;
    __TMS_UINT32                fAcceptSetup: 1;

    __TMS_USBPUMP_PROTO_VSC2_STREAM_HANDLE  hStreamOut[USBPUMP_VSC2APP_NUM_EP_OUT];
    __TMS_USBPUMP_PROTO_VSC2_STREAM_HANDLE  hStreamIn[USBPUMP_VSC2APP_NUM_EP_IN];

    __TMS_UBUFQE*               pFreeQeHeadOut;
    __TMS_UBUFQE*               pFreeQeHeadIn;

    __TMS_USBPUMP_VSC2APP_REQUEST   Requests[USBPUMP_VSC2APP_NUM_REQUEST];

    __TMS_UINT32                rSize[USBPUMP_VSC2APP_NUM_EP_OUT]; //read size
    __TMS_CHAR*                 rBuff[USBPUMP_VSC2APP_NUM_EP_OUT]; //read location
    rtems_id semReadId[USBPUMP_VSC2APP_NUM_EP_OUT];

    __TMS_UINT32                wSize[USBPUMP_VSC2APP_NUM_EP_IN]; //write size
    __TMS_CHAR*                 wBuff[USBPUMP_VSC2APP_NUM_EP_IN]; //write location
    rtems_id semWriteId[USBPUMP_VSC2APP_NUM_EP_IN];
    CALLBACK_FN usbInterfaceUpCB;
    CALLBACK_FN usbInterfaceDownCB;
};


/****************************************************************************\
|
|   API functions
|
\****************************************************************************/

__TMS_BEGIN_DECLS

extern __TMS_CONST __TMS_TEXT gk_ProtoVsc2_ObjectName[];
extern __TMS_CONST __TMS_USBPUMP_PROTO_VSC2_OUTCALL gk_UsbPumpProtoVsc2_OutCall;


__TMS_USBPUMP_VSC2APP_CONTEXT*
UsbPumpVsc2App_ClientCreate(
    __TMS_UPLATFORM*    pPlatform
);

__TMS_END_DECLS

/**** end of usbpump_vsc2app.h ****/
#endif /* _USBPUMP_VSC2APP_H_ */
