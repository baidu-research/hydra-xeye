/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _SENSOR_COMMON_H
#define _SENSOR_COMMON_H

#include "xeye_info.h"
#include "board_defines.h"
#include "board_init_bm.h"
#include "DrvI2cMaster.h"
#include "CameraDefines.h"
#include "swcFrameTypes.h"
#include "nextchip_sensor.h"
#include "ar0144_common.h"
#include "ar0330_common.h"
#include "hisi_lt8918.h"
#include "xeye_board_init.h"
#include "platform_pubdef.h"

#ifdef __cplusplus
extern "C" {
#endif
// TODO(zhoury): remove HUATU_PROJECT
#ifdef HUATU_PROJECT
#define CAM_WINDOW_START_COLUMN  0
#define CAM_WINDOW_START_ROW     0
#else
#define CAM_WINDOW_START_COLUMN  0
#define CAM_WINDOW_START_ROW     0
#endif

typedef struct {
    char* sensor_name;
    SensorType_t sensor_type;
    I2CM_Device* camera_i2c_handler;
    uint8_t mipi_channel;
    uint8_t sensor_i2c_addr;
    uint16_t chipID_reg_addr;
    uint16_t chipID_reg_value;
    uint8_t* read_protocal;
    uint8_t* write_protocal;
} SensorChipIDSkip_t;

extern GenericCamSpec* read_cam_config(XeyeBoardInfo_t* XeyeBoardInfo);

extern bool sensor_hw_reset(SensorType_t sensor_type, XeyePcbType hw_pcb_version);
extern bool sensor_soft_reset(XeyeBoardInfo_t* xeye_board_info);
#ifdef __cplusplus
}
#endif

extern uint32_t ispc_bayer_bits;
extern uint32_t ispc_debayer_bits;

extern uint32_t sensor_black_level;

extern uint32_t ispc_raw_gain_gr;
extern uint32_t ispc_raw_gain_g;
extern uint32_t ispc_raw_gain_b;
extern uint32_t ispc_raw_gain_gb;

extern uint32_t ispc_raw_clamp_0;
extern uint32_t ispc_raw_clamp_1;
extern uint32_t ispc_raw_clamp_2;
extern uint32_t ispc_raw_clamp_3;

extern uint32_t ispc_raw_output_bits;

extern uint32_t ae_luma_min;
extern uint32_t ae_luma_max;

extern uint32_t min_expo_rows;
extern uint32_t max_expo_rows;
extern uint32_t min_gain;
extern uint32_t max_gain;

extern bool XeyeGetSensorType(XeyeBoardInfo_t* XeyeBoardInfo);
// extern GenericCamSpec* XeyeReadCamSpec(Specs_t* camSpec, frameSpec* camFrameSpec,
//                                        XeyeBoardInfo_t* XeyeBoardInfo);
#endif
