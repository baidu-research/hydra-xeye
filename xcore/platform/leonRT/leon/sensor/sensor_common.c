/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include <stdio.h>
#include <DrvGpio.h>
#include "sensor_common.h"
#define MVLOG_UNIT_NAME xeye_sensor
#include <mvLog.h>

#define NUM_I2C_DEVS 3

#define HI3516_I2C_ADDRESS     (0x40 >> 1)
#define HI3516_ID_ADDR         (0x6000)
#define HI3516_ID              (0x16)

#define NEXTCHIP_ISP_ID        (0x20)
#define NEXTCHIP_SENSOR_ID     (0x2770)

#define LT8918_ID              (0x1603) 

u8 default_read_protocol[] = {S_ADDR_RD, R_ADDR_H, R_ADDR_L, DATAR, LOOP_MINUS_1};
u8 default_write_protocol[] = {S_ADDR_WR, R_ADDR_H, R_ADDR_L, DATAW, LOOP_MINUS_1};

// Default Color Combination filter
float ispcCCM[9] = {
    1.673595, -0.538250, -0.135346,
    -0.345394, 1.240305, 0.105089,
    -0.100023, -0.301202, 1.401225
};

float ispcCCMOff[3] = {
    0.00000000f, 0.00000000f, 0.00000000f
};

// Default sensor relevant ISPC settings, defferent from each kind of sensor
uint32_t ispc_bayer_bits = 10;
uint32_t ispc_debayer_bits = 10;

uint32_t sensor_black_level = 42;

uint32_t ispc_raw_gain_gr = 0x0121;
uint32_t ispc_raw_gain_g = 0x010b;
uint32_t ispc_raw_gain_b = 0x0156;
uint32_t ispc_raw_gain_gb = 0x0121;
uint32_t ispc_raw_clamp_0 = 1023;
uint32_t ispc_raw_clamp_1 = 1023;
uint32_t ispc_raw_clamp_2 = 1023;
uint32_t ispc_raw_clamp_3 = 1023;
uint32_t ispc_raw_output_bits = 10;

uint32_t ae_luma_min = 100;
uint32_t ae_luma_max = 110;

uint32_t min_expo_rows = 390;
uint32_t max_expo_rows = 1170;
uint32_t min_gain = 1;
uint32_t max_gain = 16;

const char* SensorName[] = {"SENSOR_IMX378", "SENSOR_IMX214", "SENSOR_AR0330", \
                            "SENSOR_AR0144", "HISILICON_ISP", "XCHIP_ISP", "NEXTCHIP_ISP"};

// platform 2.0 interface
bool XeyeGetSensorType(XeyeBoardInfo_t* XeyeBoardInfo) {
    SensorChipIDSkip_t Xeye2ChipIDSkip[] = {
        // AR0330
        {
            "sensor_AR0330",
            Sensor_AR0330,
            XeyeBoardInfo->i2c_info[1].handler,
            0,
            AR0330_I2C_ADDRESS,
            AR0330_ID_ADDR,
            AR0330_ID,
            default_read_protocol,
            default_write_protocol
        },
        // AR0144
        {
            "sensor_AR0144",
            Sensor_AR0144,
            XeyeBoardInfo->i2c_info[1].handler,
            0,
            AR0144_I2C_ADDRESS,
            AR0144_ID_ADDR,
            AR0144_ID,
            default_read_protocol,
            default_write_protocol
        },
        // Hisi3516
        {
            "hisi3516-mipi0",
            HiSilicon_ISP,
            XeyeBoardInfo->i2c_info[0].handler,
            1,
            HI3516_I2C_ADDRESS,
            HI3516_ID_ADDR,
            HI3516_ID,
            default_read_protocol,
            default_write_protocol
        }
    };
    SensorChipIDSkip_t XeyeFaceChipIDSkip[] = {
        // AR0330
        {
            "sensor_AR0330",
            Sensor_AR0330,
            XeyeBoardInfo->i2c_info[1].handler,
            0,
            AR0330_I2C_ADDRESS,
            AR0330_ID_ADDR,
            AR0330_ID,
            default_read_protocol,
            default_write_protocol
        },
        // AR0144
        {
            "sensor_AR0144",
            Sensor_AR0144,
            XeyeBoardInfo->i2c_info[1].handler,
            0,
            AR0144_I2C_ADDRESS,
            AR0144_ID_ADDR,
            AR0144_ID,
            default_read_protocol,
            default_write_protocol
        },
        // Hsi3516
        {
            "hisi3516-mipi",
            HiSilicon_ISP,
            XeyeBoardInfo->i2c_info[1].handler,
            0,
            HI3516_I2C_ADDRESS,
            HI3516_ID_ADDR,
            HI3516_ID,
            default_read_protocol,
            default_write_protocol
        },
        // nextchip
        // Warning: nextchip use NICP_TxPacket to get chip ID as nextchip read ID is quit different with other
        {
            "nextchip-isp",
            Nextchip_ISP,
            XeyeBoardInfo->i2c_info[1].handler,
            0,
            HI3516_I2C_ADDRESS,
            HI3516_ID_ADDR,
            HI3516_ID,
            default_read_protocol,
            default_write_protocol
        },
    };
    I2CM_StatusType status = I2CM_STAT_OK;
    SensorChipIDSkip_t* ChipIDSkip;
    int possible_num = 0;
    uint8_t bytes[2] = {0};
    uint16_t SensorChipID = 0;
    bool det_success = false;

    if (XeyeBoardInfo->hw_pcb_version == XEYE_20) {
        ChipIDSkip = Xeye2ChipIDSkip;
        possible_num = sizeof(Xeye2ChipIDSkip) / sizeof(Xeye2ChipIDSkip[0]);
    } else {
        ChipIDSkip = XeyeFaceChipIDSkip;
        possible_num = sizeof(XeyeFaceChipIDSkip) / sizeof(XeyeFaceChipIDSkip[0]);
    }

    mvLog(MVLOG_INFO, "Platform 2.0 Chip ID ship possible sensor num:%d", possible_num);

//TODO(WANGXIANG): avoid any i2c transaction to lt8918 at this moment, but it should be OK when hardware reset is ready
#ifndef HUATU_PROJECT
    for (int i = 0; i < possible_num; i++) {
        status = DrvI2cMTransaction(ChipIDSkip[i].camera_i2c_handler, ChipIDSkip[i].sensor_i2c_addr, \
                                    ChipIDSkip[i].chipID_reg_addr, ChipIDSkip[i].read_protocal, bytes, 2);

        if (status == I2CM_STAT_OK) {
            SensorChipID = (bytes[0] << 8) | bytes[1];

            if (SensorChipID == ChipIDSkip[i].chipID_reg_value) {
                mvLog(MVLOG_INFO,
                      "Platform 2.0 auto found Sensor name: %s I2CDevice :0x%x I2C addr:0x%x ID_addr:0x%x ID: 0x%x",
                      ChipIDSkip[i].sensor_name, ChipIDSkip[i].camera_i2c_handler->i2cDeviceAddr,
                      ChipIDSkip[i].sensor_i2c_addr, ChipIDSkip[i].chipID_reg_addr, ChipIDSkip[i].chipID_reg_value);
                XeyeBoardInfo->sensor_type = ChipIDSkip[i].sensor_type;
                XeyeBoardInfo->camera_i2c_handle = ChipIDSkip[i].camera_i2c_handler;
                XeyeBoardInfo->mipi_channel = ChipIDSkip[i].mipi_channel;
                det_success = true;
                break;
            }
        }
    }
 
    // TODO(wangxiang): something wrong if we check nextchip ID first
    if (det_success == false && XeyeBoardInfo->hw_pcb_version == XEYE_FACE) {
        if (lt8918_read_id(XeyeBoardInfo->i2c_info[1].handler) == LT8918_ID) {
            XeyeBoardInfo->camera_i2c_handle = XeyeBoardInfo->i2c_info[1].handler;
            XeyeBoardInfo->sensor_type = HiSilicon_ISP;
            XeyeBoardInfo->mipi_channel = 0;
            det_success = true;
        } else if (nvp2650_read_id(XeyeBoardInfo->i2c_info[1].handler) == NEXTCHIP_ISP_ID &&
                                   sensor_read_id(XeyeBoardInfo->i2c_info[1].handler) == NEXTCHIP_SENSOR_ID) {
            XeyeBoardInfo->camera_i2c_handle = XeyeBoardInfo->i2c_info[1].handler;
            XeyeBoardInfo->sensor_type = Nextchip_ISP;
            XeyeBoardInfo->mipi_channel = 0;
            det_success = true;
        }
    } else if (det_success == false && XeyeBoardInfo->hw_pcb_version == XEYE_20) {
        if (lt8918_read_id(XeyeBoardInfo->i2c_info[0].handler) == LT8918_ID) {
            XeyeBoardInfo->camera_i2c_handle = XeyeBoardInfo->i2c_info[0].handler;
            XeyeBoardInfo->sensor_type = HiSilicon_ISP;
            XeyeBoardInfo->mipi_channel = 1;
            det_success = true;
        }
    }
#else
    XeyeBoardInfo->camera_i2c_handle = XeyeBoardInfo->i2c_info[1].handler;
    XeyeBoardInfo->sensor_type = HiSilicon_ISP;
    XeyeBoardInfo->mipi_channel = 0;
    det_success = true;
#endif

    if (det_success) {
        mvLog(MVLOG_INFO, "success to detect %s ", SensorName[XeyeBoardInfo->sensor_type]);
        return true;
    } else {
        XeyeBoardInfo->sensor_type = Sensor_Unknown;
        mvLog(MVLOG_INFO, "FAIL to detect sensor");
        return false;
    }
}

GenericCamSpec* read_cam_config(XeyeBoardInfo_t* XeyeBoardInfo) {
    GenericCamSpec* CamConfig_SS_return = NULL;
    CamSpecs_t camSpec;
    camSpec.width = XeyeBoardInfo->camSpec.width;
    camSpec.height = XeyeBoardInfo->camSpec.height;
    camSpec.cam_bpp = XeyeBoardInfo->camSpec.cam_bpp;

    // if (XeyeBoardInfo->sensor_type == Sensor_Unknown) {
    if (!XeyeGetSensorType(XeyeBoardInfo)) {
        mvLog(MVLOG_ERROR, "Failed to get Sensor Type, Please Check I2C or eeprom");
        return CamConfig_SS_return;
    }

    if (XeyeBoardInfo->sensor_type == Sensor_AR0330) {
        CamConfig_SS_return = AR0330SensorGetCamConfig(camSpec.width, camSpec.height);
        // Configure ISPC params
        ispc_bayer_bits      = AR0330_ISPC_BAYER_BITS;
        ispc_debayer_bits    = AR0330_ISPC_DEBAYER_BITS;
        sensor_black_level   = AR0330_SENSOR_BLACK_LEVEL;
        ispc_raw_gain_gr     = AR0330_ISPC_RAW_GAIN_GR;
        ispc_raw_gain_g      = AR0330_ISPC_RAW_GAIN_R;
        ispc_raw_gain_b      = AR0330_ISPC_RAW_GAIN_B;
        ispc_raw_gain_gb     = AR0330_ISPC_RAW_GAIN_GB;
        ispc_raw_clamp_0     = AR0330_ISPC_RAW_CLAMP_0;
        ispc_raw_clamp_1     = AR0330_ISPC_RAW_CLAMP_1;
        ispc_raw_clamp_2     = AR0330_ISPC_RAW_CLAMP_2;
        ispc_raw_clamp_3     = AR0330_ISPC_RAW_CLAMP_3;
        ispc_raw_output_bits = AR0330_ISPC_RAW_OUTPUT_BITS;
        // Configure AEC params
        ae_luma_min = AR0330_ae_luma_min;
        ae_luma_max = AR0330_ae_luma_max;
        min_expo_rows = AR0330_min_expo_rows;
        max_expo_rows = AR0330_max_expo_rows;
        min_gain = AR0330_min_gain;
        max_gain = AR0330_max_gain;
    } else if (XeyeBoardInfo->sensor_type == Sensor_AR0144) {
        CamConfig_SS_return = AR0144SensorGetCamConfig(camSpec.width, camSpec.height);
        // Configure ISPC params
        ispc_bayer_bits      = AR0144_ISPC_BAYER_BITS;
        ispc_debayer_bits    = AR0144_ISPC_DEBAYER_BITS;
        sensor_black_level   = AR0144_SENSOR_BLACK_LEVEL;
        ispc_raw_gain_gr     = AR0144_ISPC_RAW_GAIN_GR;
        ispc_raw_gain_g      = AR0144_ISPC_RAW_GAIN_R;
        ispc_raw_gain_b      = AR0144_ISPC_RAW_GAIN_B;
        ispc_raw_gain_gb     = AR0144_ISPC_RAW_GAIN_GB;
        ispc_raw_clamp_0     = AR0144_ISPC_RAW_CLAMP_0;
        ispc_raw_clamp_1     = AR0144_ISPC_RAW_CLAMP_1;
        ispc_raw_clamp_2     = AR0144_ISPC_RAW_CLAMP_2;
        ispc_raw_clamp_3     = AR0144_ISPC_RAW_CLAMP_3;
        ispc_raw_output_bits = AR0144_ISPC_RAW_OUTPUT_BITS;
        // Configure AEC params
        ae_luma_min = AR0144_ae_luma_min;
        ae_luma_max = AR0144_ae_luma_max;
        min_expo_rows = AR0144_min_expo_rows;
        max_expo_rows = AR0144_max_expo_rows;
        min_gain = AR0144_min_gain;
        max_gain = AR0144_max_gain;
    } else if (XeyeBoardInfo->sensor_type == HiSilicon_ISP) {
        hisi_lt8918_set_sensor_format(XeyeBoardInfo->camera_i2c_handle, 
                                      XeyeBoardInfo->sensor_format);
        CamConfig_SS_return = read_hisi_lt8918_camCfg();
    } else if (XeyeBoardInfo->sensor_type == Xchip_ISP) {
        mvLog(MVLOG_ERROR, "This version not support Xchip_ISP, Read CamSpec failed");
        CamConfig_SS_return = NULL;
    } else if (XeyeBoardInfo->sensor_type == Nextchip_ISP) {
        // TODO(hyx): force format to rgb888 just for one application
        nvp2650_firmware_version(XeyeBoardInfo->camera_i2c_handle);
        nextchip_set_sensor_format(XeyeBoardInfo->camera_i2c_handle, XeyeBoardInfo->sensor_format);
        CamConfig_SS_return = read_nextchip_camCfg();
    }

    return CamConfig_SS_return;
}

bool sensor_hw_reset(SensorType_t sensor_type, XeyePcbType hw_pcb_version) {
    uint32_t sensor_hw_rst_pin = 0;
    if(sensor_type == Nextchip_ISP) {
        if (hw_pcb_version == XEYE_20) {
            sensor_hw_rst_pin = XEYE_MIPI1_RST_GPIO;
        } else {
            sensor_hw_rst_pin = XEYE_MIPI0_RST_GPIO;
        }
        DrvGpioSetMode(sensor_hw_rst_pin, D_GPIO_DIR_OUT | D_GPIO_MODE_7);
        DrvGpioSetPin(sensor_hw_rst_pin, 0);
        DrvTimerSleepMs(5);
        DrvGpioSetPin(sensor_hw_rst_pin, 1);
        // TODO(wangxiang): please check the high level time
        DrvTimerSleepMs(2000);
    }
}

bool sensor_soft_reset(XeyeBoardInfo_t* xeye_board_info) {
    SensorType_t sensor_type = xeye_board_info->sensor_type;
    I2CM_Device* i2c_handle = xeye_board_info->camera_i2c_handle;
    switch(sensor_type) {
        case Sensor_AR0330:
            AR0330SensorSoftReset(i2c_handle);
            break;
        case Sensor_AR0144:
            AR0144SensorSoftReset(i2c_handle);
            break;
        case HiSilicon_ISP:
            // TODO(hyx): add software reset function here
            break;
        case Xchip_ISP:
            // TODO(hyx): add software reset function here
            break;
        case Nextchip_ISP:
            // TODO(hyx): add software reset function here
            break;
        default:
            mvLog(MVLOG_ERROR, "Sensor type(%d) doesn't support software reset!", sensor_type);
            return false;
    }
    return true;
}
