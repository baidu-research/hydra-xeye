/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _PLATFORM_SENSOR_API_H
#define _PLATFORM_SENSOR_API_H
#include <stdio.h>
#include "sensor_common.h"
#include "platform_pubdef.h"

#define ALIGNED(x)    __attribute__((aligned(x)))
#define DDR_DATA      __attribute__((section(".ddr.data"))) ALIGNED(16)
#define DDR_BSS       __attribute__((section(".ddr.bss"))) ALIGNED(16)
#define DDR_AREA      __attribute__((section(".ddr.bss")))

#ifdef HUATU_PROJECT
#define MAX_USED_BUF             8
#else
#define MAX_USED_BUF             4
#endif

#define FIRST_INCOMING_BUF_ID    1

#ifdef __cplusplus
extern "C" {
#endif

extern  u8* RawCamBufPtr[];
extern volatile uint32_t ALIGNED(4) processingFrameCtr;
extern volatile uint32_t ALIGNED(4) newCamFrameCtr;
// TODO(zhoury): need refactor camera interface by C++ class
extern bool xeye_camera_init(CameraMode_t cam_mode, XeyeBoardInfo_t* xeye_board_info);
extern bool xeye_camera_start(CameraMode_t cam_mode, XeyeBoardInfo_t* xeye_board_info) ;
extern bool xeye_camera_stop(CameraMode_t cam_mode);
#ifdef __cplusplus
}
#endif

#endif
