/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "mv_types.h"
#include "CameraDefines.h"
#include "ar0330_common.h"
#define MVLOG_UNIT_NAME ar0330_common
#include <mvLog.h>

// List of GenericCamSpec
extern GenericCamSpec ar0330_2L_1152x768_Raw10_30Hz_camCfg;
extern GenericCamSpec ar0330_2L_1280x720_Raw10_30Hz_camCfg;
extern GenericCamSpec ar0330_2L_1920x1080_Raw10_30Hz_camCfg;

u8 ar0330_read_protocol[] = {S_ADDR_RD, R_ADDR_H, R_ADDR_L, DATAR, LOOP_MINUS_1};
u8 ar0330_write_protocol[] = {S_ADDR_WR, R_ADDR_H, R_ADDR_L, DATAW, LOOP_MINUS_1};

void AR0330SensorSoftReset(I2CM_Device* i2cHandle) {
    uint16_t camera_reg_addr = 0x301A;
    uint16_t camera_reg_value = 0x0058;
    uint8_t bytes[2] = {0};
    int statusI2c = 0;

    bytes[0] = camera_reg_value >> 8;
    bytes[1] = camera_reg_value & 0xFF;
    statusI2c = DrvI2cMTransaction(i2cHandle, AR0330_I2C_ADDRESS, camera_reg_addr,
                                   ar0330_write_protocol, bytes, 2);

    if (statusI2c == I2CM_STAT_OK) {
        mvLog(MVLOG_DEBUG,
              "Camera soft reset success! reg addr: 0x%x, value: 0x%x",
              camera_reg_addr, camera_reg_value);
    } else {
        mvLog(MVLOG_ERROR,
              "Camera soft reset failed! reg addr: 0x%x, value: 0x%x",
              camera_reg_addr, camera_reg_value);
    }

    DrvTimerSleepMs(10);
}

void AR0330SensorDumpRegister(I2CM_Device* i2cHandle, uint16_t reg_addr) {
    uint16_t camera_reg_addr = reg_addr;
    uint16_t camera_reg_value = 0;
    uint8_t bytes[2] = {0};
    int statusI2c = 0;

    statusI2c = DrvI2cMTransaction(i2cHandle, AR0330_I2C_ADDRESS, camera_reg_addr, ar0330_read_protocol,
                                   bytes, 2);
    camera_reg_value = bytes[0] << 8 | bytes[1];

    if (statusI2c == I2CM_STAT_OK) {
        mvLog(MVLOG_DEBUG,
              "Camera dump register success! reg addr: 0x%x, value: 0x%x",
              camera_reg_addr, camera_reg_value);
    } else {
        mvLog(MVLOG_ERROR,
              "Camera dump regist failed! reg addr: 0x%x, value: 0x%x",
              camera_reg_addr, camera_reg_value);
    }

    DrvTimerSleepMs(10);
}

void AR0330SensorGetFrameCount(I2CM_Device* i2cHandle) {
    uint16_t camera_reg_addr = 0x303A;
    uint16_t camera_reg_value = 0;
    uint8_t bytes[2] = {0};
    int statusI2c = 0;

    statusI2c = DrvI2cMTransaction(i2cHandle, AR0330_I2C_ADDRESS, camera_reg_addr, ar0330_read_protocol,
                                   bytes, 2);
    camera_reg_value = bytes[0] << 8 | bytes[1];

    if (statusI2c == I2CM_STAT_OK) {
        mvLog(MVLOG_DEBUG,
              "Camera get frame count success! reg addr: 0x%x, value: 0x%x",
              camera_reg_addr, camera_reg_value);

    } else {
        mvLog(MVLOG_ERROR,
              "Camera get frame count failed! reg addr: 0x%x, value: 0x%x",
              camera_reg_addr, camera_reg_value);
    }
}

void AR0330SensorSetExpo(I2CM_Device* i2cHandle, int ExpoRows) {
    uint16_t camera_reg_addr = 0x3012;
    uint16_t camera_reg_value = ExpoRows;
    uint8_t bytes[2] = {0};
    int statusI2c = 0;

    bytes[0] = camera_reg_value >> 8;
    bytes[1] = camera_reg_value & 0xFF;
    statusI2c = DrvI2cMTransaction(i2cHandle, AR0330_I2C_ADDRESS, camera_reg_addr,
                                   ar0330_write_protocol, bytes, 2);

    if (statusI2c == I2CM_STAT_OK) {
        mvLog(MVLOG_DEBUG,
              "Camera set expo success! reg addr: 0x%x, value: 0x%x",
              camera_reg_addr, camera_reg_value);
    } else {
        mvLog(MVLOG_ERROR,
              "Camera set expo failed! reg addr: 0x%x, value: 0x%x",
              camera_reg_addr, camera_reg_value);
    }

    DrvTimerSleepMs(10);
}

void AR0330SensorSetGain(I2CM_Device* i2cHandle, int Gain) {
    uint16_t camera_reg_addr = 0x3060;
    uint16_t camera_reg_value = 0;
    uint8_t bytes[2] = {0};

    camera_reg_value = Gain;

    bytes[0] = camera_reg_value >> 8;
    bytes[1] = camera_reg_value & 0xFF;
    int statusI2c = 0;

    statusI2c = DrvI2cMTransaction(i2cHandle, AR0330_I2C_ADDRESS, camera_reg_addr,
                                   ar0330_write_protocol, bytes, 2);

    if (statusI2c == I2CM_STAT_OK) {
        mvLog(MVLOG_DEBUG,
              "Camera set gain success! reg addr: 0x%x, value: 0x%x",
              camera_reg_addr, camera_reg_value);
    } else {
        mvLog(MVLOG_ERROR,
              "Camera set gain failed! reg addr: 0x%x, value: 0x%x",
              camera_reg_addr, camera_reg_value);
    }

    DrvTimerSleepMs(10);
}

GenericCamSpec* AR0330SensorGetCamConfig(uint32_t width, uint32_t height) {
    if (width == 1152 && height == 768) {
        return &ar0330_2L_1152x768_Raw10_30Hz_camCfg;
    }
    if (width == 1280 && height == 720) {
        return &ar0330_2L_1280x720_Raw10_30Hz_camCfg;
    }
    if (width == 1920 && height == 1080) {
        return &ar0330_2L_1920x1080_Raw10_30Hz_camCfg;
    }
    mvLog(MVLOG_ERROR, "Unsupported resolution: width %d, height %d", width, height);
    return NULL;
}
