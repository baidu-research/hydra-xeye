/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _AR0330_COMMON_H_
#define _AR0330_COMMON_H_
#include <DrvGpio.h>
#include "DrvTimer.h"
#include "DrvI2c.h"
#include "DrvI2cMaster.h"
#include "DrvI2cDefines.h"
#include "CameraDefines.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif
#define AR0330_I2C_ADDRESS     (0x20 >> 1)
#define AR0330_ID_ADDR          (0x3000)
#define AR0330_ID              (0x2604)

#define AR0330_ISPC_BAYER_BITS      10
#define AR0330_ISPC_DEBAYER_BITS    10

#define AR0330_SENSOR_BLACK_LEVEL   42       //From SPEC

#define AR0330_ISPC_RAW_GAIN_GR    0x0121    // 16 bits
#define AR0330_ISPC_RAW_GAIN_R     0x010b    // 16 bits
#define AR0330_ISPC_RAW_GAIN_B     0x0156    // 16 bits
#define AR0330_ISPC_RAW_GAIN_GB    0x0121    // 16 bits

#define AR0330_ISPC_RAW_CLAMP_0    1023    // 16 bits
#define AR0330_ISPC_RAW_CLAMP_1    1023    // 16 bits
#define AR0330_ISPC_RAW_CLAMP_2    1023    // 16 bits
#define AR0330_ISPC_RAW_CLAMP_3    1023    // 16 bits

#define AR0330_ISPC_RAW_OUTPUT_BITS    10

#define AR0330_ae_luma_min 100
#define AR0330_ae_luma_max 110

#define AR0330_min_expo_rows 390
#define AR0330_max_expo_rows 1170
#define AR0330_min_gain 1
#define AR0330_max_gain 16

extern float ispcCCM[9];
extern float ispcCCMOff[3];

extern u8 ar0330_read_protocol[];
extern u8 ar0330_write_protocol[];
extern void AR0330SensorDumpRegister(I2CM_Device* i2cHandle, uint16_t reg_addr);
extern GenericCamSpec* AR0330SensorGetCamConfig(uint32_t width, uint32_t height);
extern void AR0330SensorGetFrameCount(I2CM_Device* i2cHandle);
extern void AR0330SensorGetInfo(u8* slave_address, u16* id_address, u16* id_value, u8** protocal ,
                                u8* sensor_type);
extern void AR0330SensorSetExpo(I2CM_Device* i2cHandle, int ExpoRows);
extern void AR0330SensorSetGain(I2CM_Device* i2cHandle, int Gain);
extern void AR0330SensorSoftReset(I2CM_Device* i2cHandle);

#ifdef __cplusplus
}
#endif
#endif  // _AR0330_COMMON_H_
