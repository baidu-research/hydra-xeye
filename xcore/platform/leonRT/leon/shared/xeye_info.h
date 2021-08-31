/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _XEYE_INFO_H
#define _XEYE_INFO_H

#include <DrvGpio.h>
#include <stdbool.h>
#include "platform_pubdef.h"
#include "board_init_bm.h" // For NUM_I2C_DEVS

// TODO(zhoury): move all SensorType defintion to here
typedef enum SensorType {
    Sensor_IMX378 = 0,
    Sensor_IMX214 = 1,
    Sensor_AR0330 = 2,
    Sensor_AR0144 = 3,
    HiSilicon_ISP = 4,
    Xchip_ISP = 5,
    Nextchip_ISP = 6,
    Sensor_Unknown = 255
} SensorType_t;

typedef enum SensorFormat {
    Sensor_RAW16 = 0,
    ISP_YUV422 = 1, // YUV422 interleaved
    ISP_RGB888 = 2, // RGB interleaved
    ISP_YUV420P = 3, // YUV420  3 Planar mode, based on YUV422 interleaved , sub sampled in CIF
} SensorFormat_t;

typedef enum SensorResolution {
  Sensor_720p = 0,
  Sensor_1080p = 1,
  Sensor_1280_800 = 2,
  Sensor_1152_768 = 3
} SensorResolution_t;

typedef enum SensorFrameRate {
  Rate_10Fps= 10,
  Rate_25Fps= 25,
  Rate_30Fps = 30
} SensorFrameRate_t;

typedef struct Specs {
    int width;
    int height;
    int cam_bpp;
} CamSpecs_t;

typedef struct {
    XeyePcbType hw_pcb_version;
    uint8_t mac_addr[6];
    BoardI2CInfo i2c_info[NUM_I2C_DEVS];
    SensorType_t sensor_type;
    I2CM_Device* camera_i2c_handle;
    I2CM_Device* eeprom_i2c_handle;
    uint8_t mipi_channel;
    CamSpecs_t camSpec;
    SensorFormat_t sensor_format;
    uint16_t frame_buf_num;
} XeyeBoardInfo_t;

// static function
extern bool XeyeGetSensorType(XeyeBoardInfo_t* XeyeBoardInfo);
#endif
