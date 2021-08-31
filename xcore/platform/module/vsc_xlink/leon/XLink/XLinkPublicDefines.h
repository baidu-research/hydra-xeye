/*
* Copyright 2018 Intel Corporation.
* The source code, information and material ("Material") contained herein is
* owned by Intel Corporation or its suppliers or licensors, and title to such
* Material remains with Intel Corporation or its suppliers or licensors.
* The Material contains proprietary information of Intel or its suppliers and
* licensors. The Material is protected by worldwide copyright laws and treaty
* provisions.
* No part of the Material may be used, copied, reproduced, modified, published,
* uploaded, posted, transmitted, distributed or disclosed in any way without
* Intel's prior express written permission. No license under any patent,
* copyright or other intellectual property rights in the Material is granted to
* or conferred upon you, either expressly, by implication, inducement, estoppel
* or otherwise.
* Any license under such intellectual property rights must be express and
* approved by Intel in writing.
*/

///
/// @file
///
/// @brief     Application configuration Leon header
///
#ifndef _XLINKPUBLICDEFINES_H
#define _XLINKPUBLICDEFINES_H
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define USB_LINK_MAX_STREAMS 32
#define USB_LINK_MAX_PACKETS_PER_STREAM 64

typedef enum{
    X_LINK_SUCCESS = 0,
    X_LINK_ALREADY_OPEN,
    X_LINK_COMMUNICATION_NOT_OPEN,
    X_LINK_COMMUNICATION_FAIL,
    X_LINK_COMMUNICATION_UNKNOWN_ERROR,
    X_LINK_DEVICE_NOT_FOUND,
    X_LINK_TIMEOUT,
    X_LINK_ERROR
} XLinkError_t;

#define USB_LINK_INVALID_FD  (-314)

#define INVALID_STREAM_ID 0xDEADDEAD
#define INVALID_LINK_ID   0xFF

typedef uint32_t streamId_t;
typedef uint8_t linkId_t;


typedef struct streamPacketDesc_t
{
    uint8_t* data;
    uint32_t length;

} streamPacketDesc_t;

typedef struct XLinkProf_t
{
    float totalReadTime;
    float totalWriteTime;
    unsigned long totalReadBytes;
    unsigned long totalWriteBytes;
    unsigned long totalBootCount;
    float totalBootTime;
} XLinkProf_t;

typedef struct XLinkGlobalHandler_t
{
    int loglevel;
    int profEnable;
    XLinkProf_t profilingData;
} XLinkGlobalHandler_t;

typedef struct
{
    char* devicePath;
    char* devicePath2;
    int linkId;
} XLinkHandler_t;

#ifdef __cplusplus
}
#endif

#endif

/* end of include file */
