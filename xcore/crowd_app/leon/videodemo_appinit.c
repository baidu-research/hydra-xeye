///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Application configuration Leon file
///

// 1: Includes
// ----------------------------------------------------------------------------
#ifndef _USBPUMP_H_
#include "usbpump.h"
#endif /* _USBPUMP_H_ */

#include "videodemo.h"

#include "protovideo.h"
#include "usbappinit.h"
#include "usbprotoinit.h"
#include "usbvideo11.h"

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------

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
#ifndef    DESCRIPTOR_ROOT_0
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


// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
// Sections decoration is required here for downstream tools
extern CONST USBRC_ROOTTABLE gk_UsbDescriptorRoot;
extern void VideoDemo_ClientCreate(UPLATFORM* pPlatform);

// 4: Static Local Data
// ----------------------------------------------------------------------------
static USB_DATAPUMP_APPLICATION_INIT_VECTOR_FINISH_FN VideoDemoI_AppInit_VectorFinish;

// This table provides the initialization information for the protocols
// that we might load for this device.
static CONST USBPUMP_PROTOCOL_INIT_NODE InitNodes[] = {
    USBPUMP_PROTOCOL_INIT_NODE_INIT_V2(\
    /* dev class, subclass, proto */ -1, -1, -1,             \
    /* ifc class */ USB_bInterfaceClass_Video,               \
    /* subclass */ USB_bInterfaceSubClass_VideoControl,      \
    /* proto */ -1,                                          \
    /* cfg, ifc, altset */ -1, -1, -1,                       \
    /* speed */ -1,                                          \
    /* uProbeFlags */ USBPUMP_PROTOCOL_INIT_FLAG_AUTO_ADD,   \
    /* probe */ UsbPumpVideo_ProtocolProbe,                  \
    /* create */ UsbPumpVideo_ProtocolCreate,                \
    /* pQualifyAddInterface */ NULL,                         \
    /* pAddInterface */ UsbPumpVideo_ProtocolAddInterface,   \
    /* optional info */ &gk_VideoDemo_ProtoConfig            \
                                      ),
};

static CONST USBPUMP_PROTOCOL_INIT_NODE_VECTOR InitHeader =
    USBPUMP_PROTOCOL_INIT_NODE_VECTOR_INIT_V1(
        /* name of the vector */ InitNodes,
        /* prefunction */ NULL,
        /* postfunction */ NULL
    );


// This module exists solely to provide the UsbPumpApplicationInitHdr,
// which provides the glue between the abstract datapump, the chip layer,
// the descriptors, and the abstract applicaiton.
#define    INIT_VECTOR_NODE(a__UsbDescriptorRoot)            \
    USB_DATAPUMP_APPLICATION_INIT_VECTOR_INIT_V2(            \
        /* pDescriptorTable */ &a__UsbDescriptorRoot,        \
        /* DebugFlags */ UDMASK_ERRORS | UDMASK_PROTO,       \
        /* pAppProbeFunction */ NULL,                        \
        /* pAppInitFunction */                               \
            UsbPump_GenericApplicationInit_Protocols,        \
        /* pOptionalAppInfo */ &InitHeader                   \
        )

static CONST USB_DATAPUMP_APPLICATION_INIT_VECTOR UsbPumpApplicationInitVector[] = {
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
#ifndef    DESCRIPTOR_ROOT_0
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
        UsbPumpApplicationInitVector,
        /* pSetup */ NULL,
        /* pFinish */ VideoDemoI_AppInit_VectorFinish
    );


// 5: Static Function Prototypes
// ----------------------------------------------------------------------------

// 6: Functions Implementation
// ----------------------------------------------------------------------------
static void VideoDemoI_AppInit_VectorFinish(
    UPLATFORM* pPlatform,
    CONST USB_DATAPUMP_APPLICATION_INIT_VECTOR_HDR* pVecHdr,
    void* pAppInitContext,
    UINT nPorts) {
    USBPUMP_UNREFERENCED_PARAMETER(pVecHdr);
    USBPUMP_UNREFERENCED_PARAMETER(pAppInitContext);
    USBPUMP_UNREFERENCED_PARAMETER(nPorts);

    VideoDemo_ClientCreate(pPlatform);
}
