/* vscapp_appinit.c Tue Oct 14 2014 13:54:56 chwon */

/*

Module:  vscapp_appinit.c

Function:
    The firmware init vectors for the vsc application

Version:
    V3.13b  Tue Oct 14 2014 13:54:56 chwon  Edit level 4

Copyright notice:
    This file copyright (C) 2013-2014 by

        MCCI Corporation
        3520 Krums Corners Road
        Ithaca, NY  14850

    An unpublished work.  All rights reserved.

    This file is proprietary information, and may not be disclosed or
    copied without the prior permission of MCCI Corporation

Author:
    ChaeHee Won, MCCI Corporation   December 2013

Revision history:
   3.11d  Wed Dec 18 2013 10:58:53  chwon
    17949: Module created.

   3.11d  Tue Dec 31 2013 12:53:50  chwon
    17949: Add USBPUMP_PROTOCOL_INIT_FLAG_AUTO_ADD in the init node.

   3.13a  Wed Apr 09 2014 14:27:07  chwon
    18164: Use USB_DATAPUMP_APPLICATION_INIT_VECTOR_INIT_V2().

   3.13b  Tue Oct 14 2014 13:54:56  chwon
    18578: Use USBPUMP_PROTO_VSC2_CONFIG_INIT_V2().

*/

#include "usbpump_vsc2app.h"
#include "usbpumpdebug.h"

/****************************************************************************\
|
|       Manifest constants & typedefs.
|
|   This is strictly for private types and constants which will not
|   be exported.
|
\****************************************************************************/

#define EXTERN_USBRC_ROOTTABLE(NAME)    \
    extern CONST USBRC_ROOTTABLE NAME;

#ifdef DESCRIPTOR_ROOT_0
EXTERN_USBRC_ROOTTABLE(DESCRIPTOR_ROOT_0)
#endif
#ifdef DESCRIPTOR_ROOT_1
EXTERN_USBRC_ROOTTABLE(DESCRIPTOR_ROOT_1)
#endif
#ifdef DESCRIPTOR_ROOT_2
EXTERN_USBRC_ROOTTABLE(DESCRIPTOR_ROOT_2)
#endif
#ifdef DESCRIPTOR_ROOT_3
EXTERN_USBRC_ROOTTABLE(DESCRIPTOR_ROOT_3)
#endif
#ifdef DESCRIPTOR_ROOT_4
EXTERN_USBRC_ROOTTABLE(DESCRIPTOR_ROOT_4)
#endif
#ifdef DESCRIPTOR_ROOT_5
EXTERN_USBRC_ROOTTABLE(DESCRIPTOR_ROOT_5)
#endif
#ifdef DESCRIPTOR_ROOT_6
EXTERN_USBRC_ROOTTABLE(DESCRIPTOR_ROOT_6)
#endif
#ifdef DESCRIPTOR_ROOT_7
EXTERN_USBRC_ROOTTABLE(DESCRIPTOR_ROOT_7)
#endif
#ifdef DESCRIPTOR_ROOT_8
EXTERN_USBRC_ROOTTABLE(DESCRIPTOR_ROOT_8)
#endif
#ifdef DESCRIPTOR_ROOT_9
EXTERN_USBRC_ROOTTABLE(DESCRIPTOR_ROOT_9)
#endif
#ifndef DESCRIPTOR_ROOT_0
# ifndef DESCRIPTOR_ROOT_1
#  ifndef DESCRIPTOR_ROOT_2
#   ifndef DESCRIPTOR_ROOT_3
#    ifndef DESCRIPTOR_ROOT_4
#     ifndef DESCRIPTOR_ROOT_5
#      ifndef DESCRIPTOR_ROOT_6
#       ifndef DESCRIPTOR_ROOT_7
#        ifndef DESCRIPTOR_ROOT_8
#         ifndef DESCRIPTOR_ROOT_9
EXTERN_USBRC_ROOTTABLE(gk_UsbDescriptorRoot)
#         endif
#        endif
#       endif
#      endif
#     endif
#    endif
#   endif
#  endif
# endif
#endif


static USB_DATAPUMP_APPLICATION_INIT_VECTOR_FINISH_FN
UsbPumpVsc2AppI_AppInit_Finish;


/****************************************************************************\
|
|   Read-only data.
|
|   If program is to be ROM-able, these must all be tagged read-only
|   using the ROM storage class; they may be global.
|
\****************************************************************************/

CONST TEXT gk_ProtoVsc2_ObjectName[] = USBPUMP_PROTO_VSC2_NAME("sample");

static CONST USBPUMP_PROTO_VSC2_CONFIG sk_ProtoVsc2_Config =
    USBPUMP_PROTO_VSC2_CONFIG_INIT_V2(
        /* SizeControlBuffer */ 512,
        /* pObjectName */   gk_ProtoVsc2_ObjectName,
        /* DebugFlags */    0,
        /* NumMaxStreams */ 0   /* use default */
    );

/*
|| This table provides the initialization information for the protocols
|| that we might load for this device.
*/
static CONST USBPUMP_PROTOCOL_INIT_NODE sk_InitNodes[] = {
    USBPUMP_PROTOCOL_INIT_NODE_INIT_V2(\
    /* dev class, subclass, proto */ -1, -1, -1,        \
    /* ifc class */ 0xFF,                   \
    /* subclass */ 0,                   \
    /* proto */ 0,                      \
    /* cfg, ifc, altset */ -1, -1, -1,          \
    /* speed */ -1,                     \
    /* uProbeFlags */ USBPUMP_PROTOCOL_INIT_FLAG_AUTO_ADD,  \
    /* probe */ UsbPumpProtoVsc2_Probe,         \
    /* create */ UsbPumpProtoVsc2_Create,           \
    /* pQualifyAddInterface */ NULL,            \
    /* pAddInterface */ NULL,               \
    /* optional info */ &sk_ProtoVsc2_Config        \
                                      )
};

static CONST USBPUMP_PROTOCOL_INIT_NODE_VECTOR sk_InitHeader =
    USBPUMP_PROTOCOL_INIT_NODE_VECTOR_INIT_V1(
        /* name of the vector */ sk_InitNodes,
        /* prefunction */ NULL,
        /* postfunction */ NULL
    );


/*
|| This module exists solely to provide the UsbPumpApplicationInitHdr,
|| which provides the glue between the abstract DataPump, the chip layer,
|| the descriptors, and the abstract applicaiton.
*/
#define INIT_VECTOR_NODE(a__UsbDescriptorRoot)              \
    USB_DATAPUMP_APPLICATION_INIT_VECTOR_INIT_V2(           \
        /* pDescriptorTable */ &a__UsbDescriptorRoot,       \
        /* DebugFlags */ UDMASK_ERRORS | UDMASK_PROTO,      \
        /* pAppProbeFunction */ NULL,               \
        /* pAppInitFunction */                  \
            UsbPump_GenericApplicationInit_Protocols,   \
        /* pAppInfo */ &sk_InitHeader               \
        )

static CONST USB_DATAPUMP_APPLICATION_INIT_VECTOR sk_ApplicationInitVector[] = {
#ifdef DESCRIPTOR_ROOT_0
    INIT_VECTOR_NODE(DESCRIPTOR_ROOT_0),
#endif
#ifdef DESCRIPTOR_ROOT_1
    INIT_VECTOR_NODE(DESCRIPTOR_ROOT_1),
#endif
#ifdef DESCRIPTOR_ROOT_2
    INIT_VECTOR_NODE(DESCRIPTOR_ROOT_2),
#endif
#ifdef DESCRIPTOR_ROOT_3
    INIT_VECTOR_NODE(DESCRIPTOR_ROOT_3),
#endif
#ifdef DESCRIPTOR_ROOT_4
    INIT_VECTOR_NODE(DESCRIPTOR_ROOT_4),
#endif
#ifdef DESCRIPTOR_ROOT_5
    INIT_VECTOR_NODE(DESCRIPTOR_ROOT_5),
#endif
#ifdef DESCRIPTOR_ROOT_6
    INIT_VECTOR_NODE(DESCRIPTOR_ROOT_6),
#endif
#ifdef DESCRIPTOR_ROOT_7
    INIT_VECTOR_NODE(DESCRIPTOR_ROOT_7),
#endif
#ifdef DESCRIPTOR_ROOT_8
    INIT_VECTOR_NODE(DESCRIPTOR_ROOT_8),
#endif
#ifdef DESCRIPTOR_ROOT_9
    INIT_VECTOR_NODE(DESCRIPTOR_ROOT_9),
#endif
#ifndef DESCRIPTOR_ROOT_0
# ifndef DESCRIPTOR_ROOT_1
#  ifndef DESCRIPTOR_ROOT_2
#   ifndef DESCRIPTOR_ROOT_3
#    ifndef DESCRIPTOR_ROOT_4
#     ifndef DESCRIPTOR_ROOT_5
#      ifndef DESCRIPTOR_ROOT_6
#       ifndef DESCRIPTOR_ROOT_7
#        ifndef DESCRIPTOR_ROOT_8
#         ifndef DESCRIPTOR_ROOT_9
    INIT_VECTOR_NODE(gk_UsbDescriptorRoot)
#         endif
#        endif
#       endif
#      endif
#     endif
#    endif
#   endif
#  endif
# endif
#endif
};

CONST USB_DATAPUMP_APPLICATION_INIT_VECTOR_HDR gk_UsbPumpApplicationInitHdr =
    USB_DATAPUMP_APPLICATION_INIT_VECTOR_HDR_INIT_V1(
        sk_ApplicationInitVector,
        /* pSetup */    NULL,
        /* pFinish */   UsbPumpVsc2AppI_AppInit_Finish
    );


/****************************************************************************\
|
|   VARIABLES:
|
|   If program is to be ROM-able, these must be initialized
|   using the BSS keyword.  (This allows for compilers that require
|   every variable to have an initializer.)  Note that only those
|   variables owned by this module should be declared here, using the BSS
|   keyword; this allows for linkers that dislike multiple declarations
|   of objects.
|
\****************************************************************************/


static VOID
UsbPumpVsc2AppI_AppInit_Finish(
    UPLATFORM*                  pPlatform,
    CONST USB_DATAPUMP_APPLICATION_INIT_VECTOR_HDR* pVecHdr,
    VOID*                       pAppInitContext,
    UINT                        nPorts
) {
    USBPUMP_UNREFERENCED_PARAMETER(pVecHdr);
    USBPUMP_UNREFERENCED_PARAMETER(pAppInitContext);
    USBPUMP_UNREFERENCED_PARAMETER(nPorts);

    UsbPumpVsc2App_ClientCreate(pPlatform);
}

/**** end of vscapp_appinit.c ****/
