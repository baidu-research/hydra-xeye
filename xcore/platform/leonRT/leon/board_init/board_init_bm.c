/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/


// 1: Includes
// ----------------------------------------------------------------------------
#include <string.h>
#include <stdbool.h>
#include "DrvGpio.h"
#include "DrvCDCEL.h"
#include "DrvWm8325.h"
#include "DrvRegUtilsDefines.h"
#include "board_init_bm.h"
#include "boardGpioCfg.h"
#include "xeye_info.h"
#define MVLOG_UNIT_NAME board_init_bm
#include <mvLog.h>

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define IRQ_SRC_0   0
#define IRQ_SRC_1   1
#define IRQ_SRC_2   2
#define IRQ_SRC_3   3

#define NUM_I2C_DEVICES 3

#define ONE_BYTE_SIZE                    0x01
#define TWO_BYTES_SIZE                   0x02

#define PLL_STATUS_OK   0
#define I2C_STATUS_OK   0

#define EEPROM_MEMORY_SIZE               0x23
#define EEPROM_I2C_SLAVE_ADDRESS         0x50
#define EEPROM_PART_1_SIZE                 32 // only 32 bytes can be read or written at a time from eeprom
#define EEPROM_PART_2_SIZE                  3
#define EEPROM_PART_1_OFFSET             0x00
#define EEPROM_PART_2_OFFSET             0x20
#define PCB_REVISION_OFFSET              0x0D
#define BOARD_NAME_OFFSET                0x07

#ifndef WM8325_SLAVE_ADDRESS
#define WM8325_SLAVE_ADDRESS       0x36
#endif
#define WM8325_GPIOS_LEVEL_LOW   0XF000
#define GPIO_DEFAULT_CFG_VALUE   0xA400  //gpio's default value

static uint32_t boardI2CErrorHandler(I2CM_StatusType error, uint32_t param1, uint32_t param2);
static uint8_t protocolReadSample2[] = I2C_PROTO_READ_16BA;
static uint8_t protocolWriteSample2[] = I2C_PROTO_WRITE_16BA;

MV0212GpioCfg BoardGPIOCFGDefault = { .version = 0,
                                      .gpioConfig = brdGpioCfgDefault
                                    };

static tyI2cConfig i2c0DefaultConfiguration = {
    .device                = IIC1_BASE_ADR,
    .sclPin                = MV0212_I2C0_SCL_PIN,
    .sdaPin                = MV0212_I2C0_SDA_PIN,
    .speedKhz              = MV0212_I2C0_SPEED_KHZ_DEFAULT,
    .addressSize           = MV0212_I2C0_ADDR_SIZE_DEFAULT,
    .errorHandler          = &boardI2CErrorHandler,
};

static tyI2cConfig i2c1DefaultConfiguration = {
    .device                = IIC2_BASE_ADR,
    .sclPin                = MV0212_I2C1_SCL_PIN,
    .sdaPin                = MV0212_I2C1_SDA_PIN,
    .speedKhz              = MV0212_I2C1_SPEED_KHZ_DEFAULT,
    .addressSize           = MV0212_I2C1_ADDR_SIZE_DEFAULT,
    .errorHandler          = &boardI2CErrorHandler,
};

static tyI2cConfig i2c2DefaultConfiguration = {
    .device                = IIC3_BASE_ADR,
    .sclPin                = MV0212_I2C2_SCL_PIN,
    .sdaPin                = MV0212_I2C2_SDA_PIN,
    .speedKhz              = MV0212_I2C2_SPEED_KHZ_DEFAULT,
    .addressSize           = MV0212_I2C2_ADDR_SIZE_DEFAULT,
    .errorHandler          = &boardI2CErrorHandler,
};

static tyI2cConfig i2c2RevisionDetectConfiguration = {
    .device                = IIC3_BASE_ADR,
    .sclPin                = MV0212_I2C2_SCL_PIN,
    .sdaPin                = MV0212_I2C2_SDA_PIN,
    .speedKhz              = MV0212_I2C2_SPEED_KHZ_DEFAULT,
    .addressSize           = MV0212_I2C2_ADDR_SIZE_DEFAULT,
};

static BoardI2CConfig* defaultCfg[] = { &i2c0DefaultConfiguration,
                                        &i2c1DefaultConfiguration,
                                        &i2c2DefaultConfiguration
                                      };
static BoardI2CConfig* xeyeFaceI2cCfg[] = { NULL,
                                            &i2c1DefaultConfiguration,
                                            &i2c2DefaultConfiguration
                                          };

XEYEI2CCfg XeyeBoardI2CCFGDefault = {
    .version = 0,
    .i2cCfg = defaultCfg
};

XEYEI2CCfg XeyeFaceBoardI2CCFG = {
    .version = 0,
    .i2cCfg = xeyeFaceI2cCfg
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-const-variable"
static const BoardGPIOConfig brdMV0212RevDetectConfig[] = {
    // -----------------------------------------------------------------------
    // PCB Revision detect, set weak pullups on the necessary pins
    // -----------------------------------------------------------------------
    {
        9,
        9,
        ACTION_UPDATE_PAD,         // Only updating the PAD configuration
        PIN_LEVEL_LOW,             // Don't Care, not updated
        D_GPIO_MODE_0,             // Don't Care, not updated
        D_GPIO_PAD_PULL_DOWN     | // Enable weak pullups so that we can detect revision
        D_GPIO_PAD_DRIVE_2mA     |
        D_GPIO_PAD_SLEW_SLOW     |
        D_GPIO_PAD_SCHMITT_OFF   |
        D_GPIO_PAD_RECEIVER_ON   |
        D_GPIO_PAD_LOCALCTRL_OFF |
        D_GPIO_PAD_LOCALDATA_LO  |
        D_GPIO_PAD_LOCAL_PIN_IN,
        NULL
    },
    // -----------------------------------------------------------------------
    // Finally we terminate the Array
    // -----------------------------------------------------------------------
    {
        0,
        0,
        ACTION_TERMINATE_ARRAY,      // Do nothing but simply termintate the Array
        PIN_LEVEL_LOW,               // Won't actually be updated
        D_GPIO_MODE_0,               // Won't actually be updated
        D_GPIO_PAD_DEFAULTS,         // Won't actually be updated
        NULL
    }
};

static I2CM_Device i2cDevHandle[NUM_I2C_DEVICES];

static struct {
    uint32_t boardRev;
} boardCtrl;

static uint32_t boardI2CErrorHandler(I2CM_StatusType i2c_comms_error, uint32_t slave_addr,
                                     uint32_t reg_addr) {
    UNUSED(slave_addr); // hush the compiler warning.
    UNUSED(reg_addr);

    if (i2c_comms_error != I2CM_STAT_OK) {
        mvLog(MVLOG_ERROR, "I2C Error (%d) Slave (%02X) Reg (%02X)", i2c_comms_error, slave_addr, reg_addr);
    }

    return i2c_comms_error; // Because we haven't really handled the error, pass it back to the caller
}

static uint8_t verifyCheckSum(uint8_t* src, uint8_t size) {
    uint32_t sum = 0;
    uint32_t index;

    for (index = 0; index < size; index++) {
        sum = sum + *(src + index);
    }

    sum = sum & 0xFF;

    if (sum == 0) {
        return BRDCONFIG_SUCCESS; //checksum is ok
    } else {
        return BRDCONFIG_ERROR; //checksum is not ok
    }
}

static void boardInitGpio(const BoardGPIOConfig* gpio_cfg) {
    // TODO: Remove these four lines
    DrvGpioIrqSrcDisable(IRQ_SRC_0);
    DrvGpioIrqSrcDisable(IRQ_SRC_1);
    DrvGpioIrqSrcDisable(IRQ_SRC_2);
    DrvGpioIrqSrcDisable(IRQ_SRC_3);

    DrvGpioInitialiseRange(gpio_cfg);
}

static int32_t cfgWM8325GpiosDefault(I2CM_Device* dev) {
    int32_t result;
    uint8_t control_value[2];

    // Set all PMIC GPIOs to default state
    control_value[0] = ((GPIO_DEFAULT_CFG_VALUE & 0xFF00) >> 8);
    control_value[1] = (GPIO_DEFAULT_CFG_VALUE & 0x00FF);

    // TODO(zhoury): What's thre hard-coded "12" means ? Please check
    for (uint32_t i = 0; i < 12; ++i) {
        result = DrvI2cMTransaction(dev, WM8325_SLAVE_ADDRESS,
                                    (WM8325_GPIO1_CONTROL + i),
                                    protocolWriteSample2,
                                    (uint8_t*) &control_value[0],
                                    TWO_BYTES_SIZE);

        if (result != I2CM_STAT_OK) {
            mvLog(MVLOG_ERROR, "Transaction write failed reg 0x%x", (WM8325_GPIO1_CONTROL + i));
            return (int32_t)BRDCONFIG_I2C_SLAVE_ERROR;
        }
    }

    return (int32_t)BRDCONFIG_SUCCESS;
}

int32_t xeyeInitExtPll(uint32_t clock_config) {
    I2CM_Device* pll_i2c_handle;
    int          ret_val;

    // Get handle for the I2C
    pll_i2c_handle = &i2cDevHandle[2];
    // enable the CDCEL chip
    DrvGpioSetPin(58, 1);

    // Configure the PLL
    if ((ret_val = CDCE925Configure(pll_i2c_handle, clock_config))) {
        return ret_val;
    }

    return (int32_t)BRDCONFIG_SUCCESS; // Success
}

int32_t xeyeGetBrdInfo(BoardInfo* info) {
    uint8_t eeprom_data[EEPROM_MEMORY_SIZE];
    I2CM_Device i2c2_dev;
    I2CM_StatusType status;
    tyI2cConfig* i2c2_cfg;
    i2c2_cfg = &i2c2RevisionDetectConfiguration;

    status = DrvI2cMInitFromConfig(&i2c2_dev, i2c2_cfg);

    if (status != I2CM_STAT_OK) {
        mvLog(MVLOG_ERROR, "I2C initialization error");
        return (int32_t)BRDCONFIG_I2C_DRIVER_ERROR;
    }

    // read revision details from eeprom
    memset(&eeprom_data[0], 0, EEPROM_MEMORY_SIZE);
    status = DrvI2cMTransaction(&i2c2_dev, EEPROM_I2C_SLAVE_ADDRESS,
                                EEPROM_PART_1_OFFSET,
                                protocolReadSample2,
                                (uint8_t*) &eeprom_data[EEPROM_PART_1_OFFSET],
                                EEPROM_PART_1_SIZE);

    if (status != I2CM_STAT_OK) {
        //check other I2C error
        mvLog(MVLOG_ERROR, "I2C error");
        return (int32_t)BRDCONFIG_I2C_SLAVE_ERROR;
    } else {
        //read rest of EEPROM
        status = DrvI2cMTransaction(&i2c2_dev, EEPROM_I2C_SLAVE_ADDRESS,
                                    EEPROM_PART_2_OFFSET,
                                    protocolReadSample2,
                                    (uint8_t*) &eeprom_data[EEPROM_PART_2_OFFSET],
                                    EEPROM_PART_2_SIZE);

        if (status != I2CM_STAT_OK) {
            mvLog(MVLOG_ERROR, "I2C error");
            return (int32_t)BRDCONFIG_I2C_SLAVE_ERROR;
        }

        if (verifyCheckSum(&eeprom_data[0], EEPROM_MEMORY_SIZE) == BRDCONFIG_SUCCESS) {
            info->revision = eeprom_data[PCB_REVISION_OFFSET];
            memcpy(info->name, &eeprom_data[BOARD_NAME_OFFSET], NUM_CHARS_BRD_NAME + 1);
        } else {
            mvLog(MVLOG_ERROR, "Error: checksum from EEPROM is invalid");
            return (int32_t)BRDCONFIG_ERROR;
        }
    }

    return (int32_t)BRDCONFIG_SUCCESS;
}

int32_t XeyeGetPCBRevision(uint32_t* brd_rev) {
    *brd_rev = boardCtrl.boardRev;

    return BRDCONFIG_SUCCESS;
}

int32_t xeyeBoardI2CInit(XEYEI2CCfg* Board_I2C_cfg) {
    int32_t ret = BRDCONFIG_ERROR;
    uint32_t i  = 0;

    while (i < NUM_I2C_DEVS) {
        if (Board_I2C_cfg->i2cCfg[i] != NULL) {
            // Initialise the I2C device to use the I2C Hardware block
            ret = DrvI2cMInitFromConfig(&i2cDevHandle[i], Board_I2C_cfg->i2cCfg[i]);

            if (ret != I2CM_STAT_OK) {
                return BRDCONFIG_I2C_DRIVER_ERROR;
            }

            // Also setup a common error handler for I2C
            if (Board_I2C_cfg->i2cCfg[i]->errorHandler) {
                DrvI2cMSetErrorHandler(&i2cDevHandle[i], Board_I2C_cfg->i2cCfg[i]->errorHandler);
            }
        }

        i++;
    }

    return BRDCONFIG_SUCCESS;
}

int32_t xeyeGetI2CInfo(BoardI2CInfo* info, uint32_t interfaces) {
    if (info == NULL) {
        return BRDCONFIG_ERROR;
    }

    for (uint32_t i = 0; i < interfaces; ++i) {
        if (i2cDevHandle[i].i2cDeviceAddr != 0) {
            info[i].handler = &i2cDevHandle[i];
        } else {
            info[i].handler = NULL;
        }
    }

    return BRDCONFIG_SUCCESS;
}

// platform 2.0 interface
int32_t xeyeIneterfaceInit(XeyePcbType pcb_version) {
    mvLog(MVLOG_DEBUG, "platform 2.0 Xeye Ineterface Init start");
    int32_t status = BRDCONFIG_SUCCESS;

    if (pcb_version == XEYE_20) {
        status = xeyeBoardI2CInit(&XeyeBoardI2CCFGDefault);
    } else {
        status = xeyeBoardI2CInit(&XeyeFaceBoardI2CCFG);
    }

    if (status != BRDCONFIG_SUCCESS) {
        mvLog(MVLOG_ERROR, "board I2C Init error with status: %d", status);
        return BRDCONFIG_I2C_DRIVER_ERROR;
    }

    // only xeye2.0 need config WM8325
    if (pcb_version == XEYE_20) {
        // Setting all PMIC GPIOs to default configuration
        status = cfgWM8325GpiosDefault(&i2cDevHandle[2]);

        if (status != BRDCONFIG_SUCCESS) {
            mvLog(MVLOG_ERROR, "config WM8325 error with status: %d", status);
            return BRDCONFIG_I2C_SLAVE_ERROR;
        }
    }

    BoardConfigDesc config[] = {
        {
            BRDCONFIG_GPIO,
            // use this for the sake of testing as it has the same gpio config as the MV0212 R0
            (void*)brdXeye2GpioCfgDefault
        },
        {
            BRDCONFIG_END,
            NULL
        }
    };
    BoardConfigDesc* init_cfg = &config;
    int32_t conf_idx = 0;

    while (init_cfg[conf_idx].type != BRDCONFIG_END) {
        if (init_cfg[conf_idx].type == BRDCONFIG_GPIO) {
            if (init_cfg[conf_idx].config == NULL) {
                return BRDCONFIG_ERROR;
            }

            BoardGPIOConfig* board_custom_cfg = (BoardGPIOConfig*)init_cfg[conf_idx].config;
            boardInitGpio(board_custom_cfg);
        } else {
            boardInitGpio(BoardGPIOCFGDefault.gpioConfig);
        }

        conf_idx++;
    }

    mvLog(MVLOG_DEBUG, "platform 2.0 Xeye Ineterface Init end");
    return status;
}
#pragma GCC diagnostic pop
