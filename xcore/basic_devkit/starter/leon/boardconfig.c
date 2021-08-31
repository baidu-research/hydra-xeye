///
/// @file      boardconfig.c
/// @copyright All code copyright Movidius Ltd 2017, all rights reserved.
///                  For License Warranty see: common/license.txt
///
/// @brief     Board configuration
///            Platform(s) supported : ma2x8x
///

/// System Includes
/// -------------------------------------------------------------------------------------
#include <fcntl.h>
#include <rtems.h>
#include <rtems/status-checks.h>
#include <rtems/libi2c.h>
#include <rtems/libio.h>
#include <rtems/shell.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <stdlib.h>

#include <OsDrvInit.h>
#include <OsDrvGpio.h>
#include <OsDrvCpr.h>
#include <DrvDdr.h>
#include <OsDrvSvu.h>
#include <DrvSvuDefines.h>
#include <OsDrvShaveL2c.h>
#include <OsDrvCmxDma.h>
#include <DrvTimer.h>

#include <OsDrvI2cMasterBus.h>
#include <OsDrvI2cMasterMyriad.h>
#include "OsDrvI2cMasterBusInternal.h"

#include <OsDrvIOExpander_PCAL6416A.h>

#include <camSocketsMv0235.h>
#include <mvLibCameraApi.h>
#include <mvlcModulesDB.h>

#include "OsDrvMipi.h"

/// Application Includes
/// -------------------------------------------------------------------------------------
#include "boardconfig.h"
#include "XLink.h"
#include "../lib/sunny_usb_api.h"

#ifdef POWER_MEASUREMENT
#include <MV0235.h>
#include <BoardPwrMv0257Defines.h>
#include <OsDrvTempSensor.h>
#include <OsDrvAdcMcp3424.h>
#include <UnitTestApi.h>
#include "OsDrvEEPROM_24AA32A.h"
#include <MVBoardsPwr.h>
#endif
/// Source Specific #defines and types (typedef,enum,struct)
/// -------------------------------------------------------------------------------------
XLinkGlobalHandler_t ghandler;

#define MAX_SHV_NO 16
#define MAX_CNN_BLOCKS 2

// #define DEBUG_POWER_MEASUREMENT
#ifdef DEBUG_POWER_MEASUREMENT
#define PRINTFD(...)              printf(__VA_ARGS__)
#else
#define PRINTFD(...)              do { }while(0)
#endif
/// I2C
#define I2C_BUS_NAME                "/dev/i2c"
#define I2C_DEF_IRQ_PRIO            (8)
#ifdef POWER_MEASUREMENT
// Temp sensors names
#define TSENS_CSS_NAME "/dev/tempsensor0"
#define TSENS_MSS_NAME "/dev/tempsensor1"
#define TSENS_UPA0_NAME "/dev/tempsensor2"
#define TSENS_UPA1_NAME "/dev/tempsensor3"

#define TEMP_SENSORS (4)

#define SAMPLE_TIME_US (6000)
#define START_RAIL (0)
#define RAIL_MAX (28)

#define ADC_MAX (7)

#define DDR_MIN_RAIL (9)
#define DDR_MAX_RAIL (10)
#define DDR_RAIL (23)

#define UNUSED_RAIL_MIN (16)
#define USED_RAIL (20)
#define UNUSED_RAIL_MAX (22)

#define EEPROM_READ_SIZE 0x0E

#define EEPROM_PCB_REVISION_OFFSET 0x0D

#define MV0235_REV4 0x04
#define MV0235_REV3 0x03

extern OsDrvI2cMasterBusEntry mv0235_i2c_0;
extern OsDrvI2cMasterBusEntry mv0235_i2c_1;
extern OsDrvI2cMasterBusEntry mv0235_i2c_2;
extern OsDrvI2cMasterBusEntry mv0235_i2c_3;
extern OsDrvI2cMasterBusEntry mv0235_i2c_4;
extern uint8_t dssOn, openTempSensor;
int tempFd[TEMP_SENSORS];
uint32_t pcbRevision;
char devnames[TEMP_SENSORS][17] = {{TSENS_CSS_NAME}, {TSENS_MSS_NAME}, {TSENS_UPA0_NAME}, {TSENS_UPA1_NAME}};
__attribute__((aligned(4))) uint32_t busHandler = 0;
#else
DECLARE_I2C_MASTER_BUS(mv0235_i2c_0, 0, OS_DRV_I2C_MASTER_SPEED_FS, 0, I2C_DEF_IRQ_PRIO);
DECLARE_I2C_MASTER_BUS(mv0235_i2c_1, 1, OS_DRV_I2C_MASTER_SPEED_FS, 0, I2C_DEF_IRQ_PRIO);
DECLARE_I2C_MASTER_BUS(mv0235_i2c_2, 2, OS_DRV_I2C_MASTER_SPEED_FS, 0, I2C_DEF_IRQ_PRIO);
DECLARE_I2C_MASTER_BUS(mv0235_i2c_3, 3, OS_DRV_I2C_MASTER_SPEED_FS, 0, I2C_DEF_IRQ_PRIO);
DECLARE_I2C_MASTER_BUS(mv0235_i2c_4, 4, OS_DRV_I2C_MASTER_SPEED_FS, 0, I2C_DEF_IRQ_PRIO);
#endif

static int busid_I2C0;
static int busid_I2C1;
static int busid_I2C2;
static int busid_I2C3;
static int busid_I2C4;

#ifdef POWER_MEASUREMENT
const OsDrvGpioInitArray mv0235_gpio_custom_cfg = {
#else
const OsDrvGpioInitArray mv0235_gpio_default_cfg = {
#endif
    // -----------------------------------------------------------------------
    // I2C0 - I2C4 all busses' configuration
    // -----------------------------------------------------------------------
    {18, 27, {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_HIGH},
             {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_MODE_0       |
                                    OS_DRV_GPIO_DIR_OUT      |
                                    OS_DRV_GPIO_DATA_INV_OFF |
                                    OS_DRV_GPIO_WAKEUP_OFF   },
             {GPIO_CONFIG_PAD_GPIO, OS_DRV_GPIO_DEFAULT_GPIO_PAD},
     NULL
    },
    // -----------------------------------------------------------------------
    // CAM_A_C_CLK
    // -----------------------------------------------------------------------
    {44, 44, {GPIO_ACTION_NOOP,   OS_DRV_GPIO_LOW},
             {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_MODE_4       |
                                    OS_DRV_GPIO_DIR_OUT      |
                                    OS_DRV_GPIO_DATA_INV_OFF |
                                    OS_DRV_GPIO_WAKEUP_OFF   },
             {GPIO_CONFIG_PAD_GPIO, OS_DRV_GPIO_DEFAULT_GPIO_PAD},
     "44"
    },
    // -----------------------------------------------------------------------
    // CAM_A_PWDN_N
    // -----------------------------------------------------------------------
    {31, 31, {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_LOW},
             {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_MODE_7       |
                                    OS_DRV_GPIO_DIR_OUT      |
                                    OS_DRV_GPIO_DATA_INV_OFF |
                                    OS_DRV_GPIO_WAKEUP_OFF   },
             {GPIO_CONFIG_PAD_GPIO, OS_DRV_GPIO_DEFAULT_GPIO_PAD},
     "31"
    },
    // -----------------------------------------------------------------------
    // CAM_A_AUX0
    // -----------------------------------------------------------------------
    {30, 30, {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_LOW},
             {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_MODE_7       |
                                    OS_DRV_GPIO_DIR_OUT      |
                                    OS_DRV_GPIO_DATA_INV_OFF |
                                    OS_DRV_GPIO_WAKEUP_OFF   },
             {GPIO_CONFIG_PAD_GPIO, OS_DRV_GPIO_DEFAULT_GPIO_PAD},
     "30"
    },
    // -----------------------------------------------------------------------
    // CAM_B_D_CLK
    // -----------------------------------------------------------------------
    {47, 47, {GPIO_ACTION_NOOP,   OS_DRV_GPIO_LOW},
             {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_MODE_4       |
                                    OS_DRV_GPIO_DIR_OUT      |
                                    OS_DRV_GPIO_DATA_INV_OFF |
                                    OS_DRV_GPIO_WAKEUP_OFF   },
             {GPIO_CONFIG_PAD_GPIO, OS_DRV_GPIO_DEFAULT_GPIO_PAD},
     "47"
    },
    // -----------------------------------------------------------------------
    // CAM_B_PWDN_N
    // -----------------------------------------------------------------------
    {54, 54, {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_LOW},
             {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_MODE_7       |
                                    OS_DRV_GPIO_DIR_OUT      |
                                    OS_DRV_GPIO_DATA_INV_OFF |
                                    OS_DRV_GPIO_WAKEUP_OFF   },
             {GPIO_CONFIG_PAD_GPIO, OS_DRV_GPIO_DEFAULT_GPIO_PAD},
     "54"
    },
    // -----------------------------------------------------------------------
    // CAM_B_D_PWM (Reset for B1)
    // -----------------------------------------------------------------------
    {61, 61, {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_LOW},
             {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_MODE_7       |
                                    OS_DRV_GPIO_DIR_OUT      |
                                    OS_DRV_GPIO_DATA_INV_OFF |
                                    OS_DRV_GPIO_WAKEUP_OFF   },
             {GPIO_CONFIG_PAD_GPIO, OS_DRV_GPIO_DEFAULT_GPIO_PAD},
     "61"
    },
    // -----------------------------------------------------------------------
    // CAM_C_PWDN_N
    // -----------------------------------------------------------------------
    {57, 57, {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_LOW},
             {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_MODE_7       |
                                    OS_DRV_GPIO_DIR_OUT      |
                                    OS_DRV_GPIO_DATA_INV_OFF |
                                    OS_DRV_GPIO_WAKEUP_OFF   },
             {GPIO_CONFIG_PAD_GPIO, OS_DRV_GPIO_DEFAULT_GPIO_PAD},
     "57"
    },
    // -----------------------------------------------------------------------
    // CAM_D_PWDN_N
    // -----------------------------------------------------------------------
    {58, 58, {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_LOW},
             {GPIO_ACTION_UPDATE,   OS_DRV_GPIO_MODE_7       |
                                    OS_DRV_GPIO_DIR_OUT      |
                                    OS_DRV_GPIO_DATA_INV_OFF |
                                    OS_DRV_GPIO_WAKEUP_OFF   },
             {GPIO_CONFIG_PAD_GPIO, OS_DRV_GPIO_DEFAULT_GPIO_PAD},
     "58"
    },
    // -----------------------------------------------------------------------
    // Finally we terminate the Array
    // -----------------------------------------------------------------------
    OS_DRV_GPIO_ARRAY_TERMINATOR
};

static const OsDrvIoExpanderGpioInit expander_gpios_0x20 = {
    .i2cBusNo = 0,
    .i2cDeviceAddress = 0x20,
    .interrupt_pin = "/dev/gpio.pcal_int",
    .interruptPriority = 1,
    .gpios =
    {
        OS_DRV_IOEXPANDER_ENTRY(0, 0, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port0_pin0"),
        OS_DRV_IOEXPANDER_ENTRY(0, 1, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port0_pin1"),
        OS_DRV_IOEXPANDER_ENTRY(0, 2, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port0_pin2"),
        OS_DRV_IOEXPANDER_ENTRY(0, 3, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port0_pin3"),
        OS_DRV_IOEXPANDER_ENTRY(0, 4, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port0_pin4"),
        OS_DRV_IOEXPANDER_ENTRY(0, 5, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port0_pin5"),
        OS_DRV_IOEXPANDER_ENTRY(0, 6, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port0_pin6"),
        OS_DRV_IOEXPANDER_ENTRY(0, 7, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port0_pin7"),
        OS_DRV_IOEXPANDER_ENTRY(1, 0, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port1_pin0"),
        OS_DRV_IOEXPANDER_ENTRY(1, 1, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port1_pin1"),
        OS_DRV_IOEXPANDER_ENTRY(1, 2, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port1_pin2"),
        OS_DRV_IOEXPANDER_ENTRY(1, 3, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port1_pin3"),
        OS_DRV_IOEXPANDER_ENTRY(1, 4, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port1_pin4"),
        OS_DRV_IOEXPANDER_ENTRY(1, 5, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port1_pin5"),
        OS_DRV_IOEXPANDER_ENTRY(1, 6, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port1_pin6"),
        OS_DRV_IOEXPANDER_ENTRY(1, 7, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x20_port1_pin7"),
    }
};

static const OsDrvIoExpanderGpioInit expander_gpios_0x21 = {
    .i2cBusNo = 0,
    .i2cDeviceAddress = 0x21,
    .interrupt_pin = "/dev/gpio.pcal_int",
    .interruptPriority = 1,
    .gpios =
    {
        OS_DRV_IOEXPANDER_ENTRY(0, 0, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x21_port0_pin0"),
        OS_DRV_IOEXPANDER_ENTRY(0, 1, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x21_port0_pin1"),
        OS_DRV_IOEXPANDER_ENTRY(0, 2, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x21_port0_pin2"),
        OS_DRV_IOEXPANDER_ENTRY(0, 3, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x21_port0_pin3"),
        OS_DRV_IOEXPANDER_ENTRY(0, 4, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x21_port0_pin4"),
        OS_DRV_IOEXPANDER_ENTRY(0, 5, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x21_port0_pin5"),
        OS_DRV_IOEXPANDER_ENTRY(0, 6, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x21_port0_pin6"),
        OS_DRV_IOEXPANDER_ENTRY(0, 7, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00,
                                   "ioexp_0x21_port0_pin7"),
        OS_DRV_IOEXPANDER_ENTRY(1, 0, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00, "CAM_C_AUX_IO2"),
        OS_DRV_IOEXPANDER_ENTRY(1, 1, GPIO_OUTPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00, "CAM_C_AUX_IO1"),
        OS_DRV_IOEXPANDER_ENTRY(1, 2, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00, "CAM_B_AUX_IO2"),
        OS_DRV_IOEXPANDER_ENTRY(1, 3, GPIO_OUTPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00, "CAM_B_AUX_IO1"),
        OS_DRV_IOEXPANDER_ENTRY(1, 4, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00, "CAM_A_AUX_IO2"),
        OS_DRV_IOEXPANDER_ENTRY(1, 5, GPIO_OUTPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00, "CAM_A_AUX_IO1"),
        OS_DRV_IOEXPANDER_ENTRY(1, 6, GPIO_INPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00, "CAM_D_AUX_IO2"),
        OS_DRV_IOEXPANDER_ENTRY(1, 7, GPIO_OUTPUT, 0,
                                   OS_DRV_IOEXPANDER_DRIVE_1_00, "CAM_D_AUX_IO1"),
    }
};

static OsDrvCprAuxClockConfig aux_clocks[] =
{
#ifdef POWER_MEASUREMENT
    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_TSENS,   //temp sensor
        .clkSource = OS_DRV_CPR_CLK_REF0,
        .numerator = 1,
        .denominator = 20,
    },
#endif
    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_GPIO0,
        .clkSource = OS_DRV_CPR_CLK_REF0, /// 24 Mhz on mv235
        .numerator = 1,
        .denominator = 1, /// 24 Mhz
    },
    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_GPIO1,
        .clkSource = OS_DRV_CPR_CLK_REF0, /// 24 Mhz on mv250
        .numerator = 1,
        .denominator = 1, /// 24 Mhz
    },
    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_MIPI_ECFG, // 24 MHz
        .clkSource = OS_DRV_CPR_CLK_REF0,
        .numerator = 1,
        .denominator = 1
    },
    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_MIPI_CFG, // 24 MHz
        .clkSource = OS_DRV_CPR_CLK_REF0,
        .numerator = 1,
        .denominator = 1
    },
    {   .device = OS_DRV_CPR_DEV_CSS_AUX_MIPI_TX0,  // 24 MHz
        .clkSource = OS_DRV_CPR_CLK_REF0,
        .numerator = 1,
        .denominator = 1,
    },
    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_MIPI_TX1, // 24 MHz
        .clkSource = OS_DRV_CPR_CLK_REF0,
        .numerator = 1,
        .denominator = 1
    },

    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_USB_PHY_REF_ALT,
        .clkSource = OS_DRV_CPR_CLK_PLL0,
        .numerator = 1,
        .denominator = 25
    },
    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_USB_CTRL_SUSPEND,
        .clkSource = OS_DRV_CPR_CLK_PLL0,
        .numerator = 1,
        .denominator = 10
    },
    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_USB20_REF,
        .clkSource = OS_DRV_CPR_CLK_PLL0,
        .numerator = 3,
        .denominator = 175
    },
    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_USB_CTRL_REF,
        .clkSource = OS_DRV_CPR_CLK_PLL0,
        .numerator = 1,
        .denominator = 600
    },
    {
        .device = OS_DRV_CPR_DEV_CSS_AUX_USB_CTRL,
        .clkSource = OS_DRV_CPR_CLK_PLL0,
        .numerator = 1,
        .denominator = 2
    },
    OS_DRV_CPR_AUX_ARRAY_TERMINATOR
};

static OsDrvCprDeviceConfig dev_enables[] =
{
    {OS_DRV_CPR_DEV_CSS_I2C0, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_CSS_I2C1, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_CSS_I2C2, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_CSS_I2C3, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_CSS_I2C4, OS_DRV_CPR_DEV_ENABLE},

    {OS_DRV_CPR_DEV_MSS_ISP_MIPI_RX0, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_MIPI_RX1, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_MIPI_RX2, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_MIPI_RX3, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_MIPI_RX4, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_MIPI_RX5, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_MIPI_RX6, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_MIPI_RX7, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_MIPI_TX0, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_MIPI_TX1, OS_DRV_CPR_DEV_ENABLE},

    {OS_DRV_CPR_DEV_MSS_CNN0, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_CNN1, OS_DRV_CPR_DEV_ENABLE},

    {OS_DRV_CPR_DEV_UPA_SHAVE_0, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_1, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_2, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_3, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_4, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_5, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_6, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_7, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_8, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_9, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_10, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_11, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_12, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_13, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_14, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_15, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_SHAVE_L2, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_CDMA, OS_DRV_CPR_DEV_ENABLE},

    //MSS ISP entries
    {OS_DRV_CPR_DEV_MSS_ISP_SIGMA, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_LSC, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_RAW, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_LCA, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_DEBAYER, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_DOGL, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_LUMA, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_SHARPEN, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_CGEN, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_MEDIAN, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_CHROMA, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_CC, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_LUT, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_UPFIRDN0, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_UPFIRDN1, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_MSS_ISP_UPFIRDN2, OS_DRV_CPR_DEV_ENABLE},

    {OS_DRV_CPR_DEV_CSS_AUX_USB_PHY_REF_ALT, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_CSS_AUX_USB_CTRL_SUSPEND, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_CSS_AUX_USB20_REF, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_CSS_AUX_USB_CTRL_REF, OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_CSS_AUX_USB_CTRL, OS_DRV_CPR_DEV_ENABLE},

    {OS_DRV_CPR_DEV_CSS_USB,   OS_DRV_CPR_DEV_ENABLE},
    {OS_DRV_CPR_DEV_UPA_MTX, OS_DRV_CPR_DEV_ENABLE},

    OS_DRV_CPR_DEV_ARRAY_TERMINATOR
};

static OsDrvCprConfig cprconfig = {
    .auxCfg = aux_clocks,
    .devCfg = dev_enables
};
#ifdef POWER_MEASUREMENT
BoardPwrRailCfg cfg = { BOARD_PWR_MODE_CONTINUOUS,
                        BOARD_PWR_ACCURACY_12BITS,
                        BOARD_PWR_GAIN_X1};
#endif
OS_DRV_INIT_CPR_CFG_DEFINE(&cprconfig);

/// Global Data (Only if absolutely necessary)
/// -------------------------------------------------------------------------------------

/// System Clock configuration on start-up
BSP_SET_CLOCK(DEFAULT_OSC0_KHZ, PLL_DESIRED_FREQ_KHZ, 1, 1, \
           DEFAULT_RTEMS_CSS_LOS_CLOCKS, DEFAULT_RTEMS_MSS_LRT_CLOCKS, DEFAULT_UPA_CLOCKS, 0, 0);


/// Program L2 cache behaviour
BSP_SET_L2C_CONFIG(1, L2C_REPL_LRU, 0, /*DEFAULT_RTEMS_L2C_MODE*/ L2C_MODE_WRITE_THROUGH, 0, NULL);

//OS_DRV_INIT_SHAVE_L2C_MODE_DEFINE(OS_DRV_SHAVE_L2C_BYPASS);
OS_DRV_INIT_SHAVE_L2C_MODE_DEFINE(OS_DRV_SHAVE_L2C_NORMAL);

/// Static Local Data
/// -------------------------------------------------------------------------------------

/// Static Function Prototypes
/// -------------------------------------------------------------------------------------

/// Functions Implementation
/// -------------------------------------------------------------------------------------
static int initClocksAndMemory(void)
{

    int sc = RTEMS_SUCCESSFUL;

    /* reset L2C configuration */
    sc = OsDrvShaveL2cResetAllPart();
    if (sc)
        assert(0);

    //We'll configure 2 partitions: one for code and one for data
    uint8_t partID[4];
    /* Set Shave L2 cache partitions 128K */
    sc = OsDrvShaveL2cAddPart(OS_DRV_SHAVE_L2C_PART_64_KB, 0, 1, &partID[0]);
    sc += OsDrvShaveL2cAddPart(OS_DRV_SHAVE_L2C_PART_64_KB, 0, 1, &partID[1]);
    sc += OsDrvShaveL2cAddPart(OS_DRV_SHAVE_L2C_PART_64_KB, 0, 1, &partID[2]);
    sc += OsDrvShaveL2cAddPart(OS_DRV_SHAVE_L2C_PART_64_KB, 0, 1, &partID[3]);

    if (sc)
        assert(0);

    //Invalidate cache
    OsDrvShaveL2cFlushInvPart(partID[0], OS_DRV_SHAVE_L2C_INV);
    OsDrvShaveL2cFlushInvPart(partID[1], OS_DRV_SHAVE_L2C_INV);
    OsDrvShaveL2cFlushInvPart(partID[2], OS_DRV_SHAVE_L2C_INV);
    OsDrvShaveL2cFlushInvPart(partID[3], OS_DRV_SHAVE_L2C_INV);

    unsigned int cmx_isi_priority = 0x000f0000;
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY0_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY1_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY2_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY3_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY4_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY5_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY6_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY7_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY8_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY9_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY10_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY11_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY12_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY13_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY14_CTRL, cmx_isi_priority);
    SET_REG_WORD(CMX_SLICE_ISI_PRIORITY15_CTRL, cmx_isi_priority);

    uint32_t regVal;
    regVal = GET_REG_WORD_VAL(MSS_SUPER_WR_PRI_ADR);
    regVal |= (0x1 << 0x1B);
    regVal |= (0x1 << 0x1C);  // Add CNN 1 write port to this mask
    SET_REG_WORD(MSS_SUPER_WR_PRI_ADR, regVal);
    regVal = GET_REG_WORD_VAL(MSS_SUPER_WR_PRI_ADR);
    SET_REG_WORD(CMX_SLICE_LC_PRIORITY0_CTRL, 0x30303030);
    SET_REG_WORD(CMX_SLICE_LC_PRIORITY1_CTRL, 0x30303030);
    SET_REG_WORD(CMX_SLICE_LC_PRIORITY2_CTRL, 0x30303030);
    SET_REG_WORD(CMX_SLICE_LC_PRIORITY3_CTRL, 0x30303030);
    SET_REG_WORD(CMX_SLICE_LC_PRIORITY4_CTRL, 0x30303030);

    OsDrvCmxDmaSetupStruct configInt =
    { 14, // irq_priority
      1  // irq_enable (source 1 on Leon OS - but all enabled by default)
    };
    sc = OsDrvCmxDmaInitialize(&configInt);
    if (sc)
        assert(0);

    //Initialize the SHAVE driver
    sc = OsDrvSvuInit();
    if (sc)
        assert(0);

    for(int i = 0; i <= 15; i++)
    {
        sc = OsDrvCprTurnOnIsland((HglCprPwrIsland)(HGL_CPR_PWR_ISLAND_SHAVE_0 + i));
        assert(sc == OS_MYR_DRV_SUCCESS);

        OsDrvCprDevice shave_dev = (OsDrvCprDevice)(OS_DRV_CPR_DEV_UPA_SHAVE_0 + i);
        OsDrvCprDeviceConfig devCfg = { shave_dev, OS_DRV_CPR_DEV_ENABLE };
        sc = OsDrvCprSysDeviceAction(&devCfg);

        if (sc == RTEMS_UNSATISFIED)
        {
            devCfg.device = shave_dev;
            devCfg.action = OS_DRV_CPR_DEV_RESET;
            sc = OsDrvCprSysDeviceAction(&devCfg);
            assert(sc == OS_MYR_DRV_SUCCESS);
        }
        else
        {
            assert(sc == OS_MYR_DRV_SUCCESS);
        }
    }

    // DEASSERT_RESET cnn blocks
    OsDrvCprDeviceConfig devConfig;
    for(int i = 0; i < MAX_CNN_BLOCKS; i++)
    {
        devConfig.device = OS_DRV_CPR_DEV_MSS_CNN0 + i;
        devConfig.action = OS_DRV_CPR_DEV_ENABLE;

        sc = OsDrvCprSysDeviceAction(&devConfig);
        if (sc)
            return sc;
    }

    return sc;
}

static rtems_status_code brdInitCpr(void)
{
    uint32_t pll0, senA_mclk, senB_mclk, mipi_cfg, mipi_ecfg;

    OsDrvCprGetClockFrequency(OS_DRV_CPR_CLK_PLL0,&pll0);
    OsDrvCprGetAuxClockFrequency(OS_DRV_CPR_DEV_CSS_AUX_GPIO0, &senA_mclk);
    OsDrvCprGetAuxClockFrequency(OS_DRV_CPR_DEV_CSS_AUX_GPIO0, &senB_mclk);
    OsDrvCprGetAuxClockFrequency(OS_DRV_CPR_DEV_CSS_AUX_MIPI_CFG, &mipi_cfg);
    OsDrvCprGetAuxClockFrequency(OS_DRV_CPR_DEV_CSS_AUX_MIPI_ECFG, &mipi_ecfg);

    printf ( "PLL0: %lu AUX_IO0: %lu AUX_IO1: %lu MCFG: %lu MECFG: %lu\n",
        pll0, senA_mclk, senB_mclk, mipi_cfg, mipi_ecfg);

    return RTEMS_SUCCESSFUL;
}

static rtems_status_code brdInitGpios(void)
{
    rtems_status_code sc;
#ifdef POWER_MEASUREMENT
    sc = OsDrvGpioInit(mv0235_gpio_custom_cfg);
#else
    sc = OsDrvGpioInit(mv0235_gpio_default_cfg);
#endif

    RTEMS_CHECK_SC(sc, "rtems_io_initialize\n");

    return sc;
}

static rtems_status_code brdInitI2c(void)
{
    rtems_status_code sc;

    sc = rtems_libi2c_initialize();
    RTEMS_CHECK_SC(sc, "rtems_libi2c_initialize FAILED %d \n");

    /////////////////////////// I2C BUS 0 ///////////////////////////////
    busid_I2C0 = rtems_libi2c_register_bus(I2C_BUS_NAME, (rtems_libi2c_bus_t *)&mv0235_i2c_0);
    RTEMS_CHECK_RV(busid_I2C0, "register the i2cBus0 bus\n");
    printf("i2cBus0 id=%d\n", busid_I2C0);

    /////////////////////////// I2C BUS 1 ///////////////////////////////
    busid_I2C1 = rtems_libi2c_register_bus(I2C_BUS_NAME, (rtems_libi2c_bus_t *)&mv0235_i2c_1);
    RTEMS_CHECK_RV(busid_I2C1, "register the i2cBus1 bus\n");
    printf("i2cBus1 id=%d\n", busid_I2C1);

    /////////////////////////// I2C BUS 2 ///////////////////////////////
    busid_I2C2 = rtems_libi2c_register_bus(I2C_BUS_NAME, (rtems_libi2c_bus_t *)&mv0235_i2c_2);
    RTEMS_CHECK_RV(busid_I2C2, "register the i2cBus2 bus\n");
    printf("i2cBus2 id=%d\n", busid_I2C2);

    /////////////////////////// I2C BUS 3 ///////////////////////////////
    busid_I2C3 = rtems_libi2c_register_bus(I2C_BUS_NAME, (rtems_libi2c_bus_t *)&mv0235_i2c_3);
    RTEMS_CHECK_RV(busid_I2C3, "register the i2cBus3 bus\n");
    printf("i2cBus3 id=%d\n", busid_I2C3);

    /////////////////////////// I2C BUS 4 ///////////////////////////////
    busid_I2C4 = rtems_libi2c_register_bus(I2C_BUS_NAME, (rtems_libi2c_bus_t *)&mv0235_i2c_4);
    RTEMS_CHECK_RV(busid_I2C4, "register the i2cBus4 bus\n");
    printf("i2cBus4 id=%d\n", busid_I2C4);

    /// Allocate the buses
    sc = rtems_libi2c_ioctl(RTEMS_LIBI2C_MAKE_MINOR(busid_I2C0,0),
                            OS_DRV_I2C_MASTER_INTERNAL_ALLOC_BUS);
    RTEMS_CHECK_RV(sc, "allocate the i2cBus0 bus\n");
    sc = rtems_libi2c_ioctl(RTEMS_LIBI2C_MAKE_MINOR(busid_I2C1,0),
                            OS_DRV_I2C_MASTER_INTERNAL_ALLOC_BUS);
    RTEMS_CHECK_RV(sc, "allocate the i2cBus1 bus\n");
    sc = rtems_libi2c_ioctl(RTEMS_LIBI2C_MAKE_MINOR(busid_I2C2,0),
                            OS_DRV_I2C_MASTER_INTERNAL_ALLOC_BUS);
    RTEMS_CHECK_RV(sc, "allocate the i2cBus2 bus\n");
    sc = rtems_libi2c_ioctl(RTEMS_LIBI2C_MAKE_MINOR(busid_I2C3,0),
                            OS_DRV_I2C_MASTER_INTERNAL_ALLOC_BUS);
    RTEMS_CHECK_RV(sc, "allocate the i2cBus3 bus\n");
    sc = rtems_libi2c_ioctl(RTEMS_LIBI2C_MAKE_MINOR(busid_I2C4,0),
                            OS_DRV_I2C_MASTER_INTERNAL_ALLOC_BUS);
    RTEMS_CHECK_RV(sc, "allocate the i2cBus4 bus\n");

    return RTEMS_SUCCESSFUL;
}

rtems_status_code brdInitAuxDevices(void)
{
    rtems_status_code status = OsDrvCprTurnOnIsland(OS_DRV_CPR_PWR_ISLAND_USB);
    if (status)
        return (status);

    /// Init gpio expanders
    rtems_status_code sc;
    rtems_device_major_number gpioExpanderMajor;

    sc = rtems_io_register_driver(0, &os_drv_ioexpander_PCAL6416A_drv_tbl,
                                  &gpioExpanderMajor);
    RTEMS_CHECK_SC(sc, "register os_drv_ioexpander_PCAL6416A_drv_tbl\n");
    if(sc)
    {
        return sc;
    }

    /// Expander #0
    sc = rtems_io_initialize(gpioExpanderMajor, 0, (void *)&expander_gpios_0x20);
    RTEMS_CHECK_SC(sc, "io_initialize expander_gpios_0x20\n");
    if(sc)
    {
        return sc;
    }

    /// Expander #1
    sc = rtems_io_initialize(gpioExpanderMajor, 0, (void *)&expander_gpios_0x21);
    RTEMS_CHECK_SC(sc, "io_initialize expander_gpios_0x21\n");
    if(sc)
    {
        return sc;
    }

    return RTEMS_SUCCESSFUL;
}

rtems_status_code brdInitCameras(void)
{
    rtems_status_code sc;

    sc = mvlcRegisterCameras(&camSocketListMv0235, &mvlcModulesDataBase, "Camera_", MVLC_PROBE_ALL);
    printf("Detected cameras: \n");
    uint32_t i;
    for(i=0; i < camSocketListMv0235.count; i++)
    {
        if(camSocketListMv0235.camsocketsList[i]->locked_to != NULL)
        {
            int fd = open(camSocketListMv0235.camsocketsList[i]->locked_to, O_RDWR);
            mvlcCameraFeatures_t* cfeat = malloc(sizeof(mvlcCameraFeatures_t));
            sc = ioctl(fd, MVLC_IOCTL_CAM_GET_FEATURES, (void*)cfeat);
            if(sc)
            {
                free(cfeat);
                close(fd);
                return sc;
            }
            close(fd);
            printf ( "  %s on socket %s. Camera type: %s\n",
                    camSocketListMv0235.camsocketsList[i]->locked_to,
                    camSocketListMv0235.camsocketsList[i]->socketname,
                    cfeat->moduleName );
            free(cfeat);
        }
    }

    return sc;
}
#ifdef POWER_MEASUREMENT
rtems_status_code initPwrDriver(uint32_t pcbRevision){
    rtems_status_code sc;
    char adc_name[] = BOARD_PWR_MCP3424_DEVNAME_0;
    int adcMinor=0;

    for(uint32_t i = 0; i < ADC_MAX; ++i) {
        // Register ADC devices on I2C0
        adcMinor = rtems_libi2c_register_drv(
                adc_name,
                &os_drv_adc_mcp3424_protocol_drv_tbl,
                busHandler,
                BOARD_PWR_MCP3424_I2C_ADR_0 + i);
        assert(adcMinor > 0);
        PRINTFD("Successfully registered i2c rtems drv for device %s\n", adc_name);
        adc_name[strlen(adc_name) - 1]= ((adc_name[strlen(adc_name) - 1] + 1) & 0x3F);
    }

    // Init the board power measurement driver
    sc = BoardPwrInit(pcbRevision);
    assert(sc == RTEMS_SUCCESSFUL);

    // Test all the rails
    for (int8_t i = START_RAIL; i < RAIL_MAX; ++i) {
        if((i >= UNUSED_RAIL_MIN && i < USED_RAIL) || ((i > USED_RAIL && i <= UNUSED_RAIL_MAX) &&  \
           ((pcbRevision == MV0235_REV4) || pcbRevision == MV0235_REV3)))
        {
            //Skip over unneeded rails
            continue;
        }
        // Select the ith rail
        sc = BoardPwrSetRailCfg(&cfg, i);
        assert(sc == RTEMS_SUCCESSFUL);
    }
    return sc;
}

rtems_status_code initTempDriver(void){
    int32_t sc;
    rtems_device_major_number temp_sens_major;

    struct OsDrvTempSensorConfig initcfg =
    {
        .irq_priority = 1,
        {
            {OS_DRV_TEMP_SENS_CONT_TEMPERATURE},  // CSS sensor
            {OS_DRV_TEMP_SENS_CONT_TEMPERATURE},  // MSS sensor
            {OS_DRV_TEMP_SENS_CONT_TEMPERATURE},  // UPA0 sensor
            {OS_DRV_TEMP_SENS_CONT_TEMPERATURE}   // UPA1 sensor
        }
    };
    sc = rtems_io_register_driver(0, &temp_sensor_drv_tbl, &temp_sens_major);
    if (sc) {
        printf("Error registering the driver\n");
        unitTestLogFail();
    }
    else {
        sc = rtems_io_initialize(temp_sens_major, 0, (void *)&initcfg);
        if (sc) {
            printf("Error initializing the driver\n");
            unitTestLogFail();
        }
        else {
            for(int i=0;i<TEMP_SENSORS;i++){
                tempFd[i] = open(devnames[i], O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (tempFd[i] < 0) {
                    printf("Open failed for %s with error code %d\n",devnames[i],tempFd[i]);
                    unitTestLogFail();
                    break;
                }
            }
        }
    }
    return sc;
}
rtems_status_code readTemp(void ){
    int sc;
    float tempValue[TEMP_SENSORS];
    for(int i=0;i<TEMP_SENSORS;i++){
        sc = ioctl(tempFd[i], IOCTL_READ_TEMP, &tempValue[i]);
        if (sc){
            printf("read ioctl failed for %s with error code %d\n",devnames[i],sc);
            unitTestLogFail();
            break;
        }
    }
    if(sc != RTEMS_SUCCESSFUL){
        return sc;
    }else{
        printf("Temperature CSS=%.2f%c MSS=%.2f%c UPA0=%.2f%c UPA1=%.2f%c \n",
                tempValue[0],0xB0, tempValue[1],0xB0, tempValue[2],0xB0, tempValue[3],0xB0);
        return RTEMS_SUCCESSFUL;
    }
}

rtems_status_code readPower(void) {
    float sample;
    float corePower=0, ddrPower=0;
    int sc;
    for (int8_t i = START_RAIL; i < RAIL_MAX; ++i) {
        if((i >= UNUSED_RAIL_MIN && i < USED_RAIL) || ((i > USED_RAIL && i <= UNUSED_RAIL_MAX) &&  \
           ((pcbRevision == MV0235_REV4) || pcbRevision == MV0235_REV3)))
        {
            //Skip over unneeded rails
            continue;
        }
        // Start sampling the ith rail
        sc = BoardPwrStartSamplingRail(i);
        assert(sc == RTEMS_SUCCESSFUL);
        // Wait for the ADC to complete the sampling
        usleep(SAMPLE_TIME_US);
        sc = BoardPwrReadPowerSample(i, &sample);
        switch(sc){
            case RTEMS_SUCCESSFUL:
                if ((i>=DDR_MIN_RAIL && i<=DDR_MAX_RAIL) || i==DDR_RAIL){  //DRAM rails
                ddrPower+=sample;
                }
                else{
                corePower+=sample;
                }
                PRINTFD("rail n%d, P=%0.2fmW\n",i, sample);
                break;
            case RTEMS_IO_ERROR:
                PRINTFD("ADC error.\n");
                unitTestLogFail();
                return sc;
                break;
            case RTEMS_INVALID_ADDRESS:
                PRINTFD("Sample pointer set to NULL.\n");
                unitTestLogFail();
                return sc;
                break;
            case RTEMS_NOT_DEFINED:
                PRINTFD("Rail not supported.\n");
                return sc;
                break;
            case RTEMS_NOT_CONFIGURED:
                PRINTFD("This is not the rail we are looking for.\n");
                unitTestLogFail();
                return sc;
                break;
            case RTEMS_UNSATISFIED:
                PRINTFD("Sample not ready.\n");
                unitTestLogFail();
                return sc;
            default:
                return sc;
                break;
        }
    }
    printf("Core Power=%0.2fmW  DDR Power=%0.2fmW ",corePower, ddrPower);
    return RTEMS_SUCCESSFUL;
}

int32_t revisionDetect(){
    int fd, eepromMinor = 0, err;
    uint32_t revision;

    //eeprom is needed for revision detect
    eepromMinor = rtems_libi2c_register_drv(EEPROM_CHIP_NAME,
                                            &eeprom_24aa32a_protocol_drv_tbl,
                                            busHandler,
                                            EEPROM_SLAVE_ADDRESS);
    if (eepromMinor <= 0) {
        PRINTFD("rtems_libi2c_register_drv failed %s minor %d Addr %x\n",
                EEPROM_CHIP_NAME,
                eepromMinor,
                EEPROM_SLAVE_ADDRESS);
        return eepromMinor;
    }
    fd = open(I2C_BUS_NAME "." EEPROM_CHIP_NAME, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0){
        PRINTFD("Error: open failed %s \n",
                EEPROM_CHIP_NAME);
        unitTestLogFail();
        return fd;
    }

    err = lseek(fd, 0, SEEK_SET);
    if (err < 0){
        PRINTFD("Error: file reposition failed\n");
        unitTestLogFail();
        return err;
    }
    uint8_t *eepromBuffer = (uint8_t *) malloc(EEPROM_READ_SIZE);
    if(eepromBuffer==NULL){
        PRINTFD("Error: memory allocation failed\n");
        unitTestLogFail();
        return -1;
    }
    memset(eepromBuffer, 0, EEPROM_READ_SIZE);
    err = read(fd, eepromBuffer, EEPROM_READ_SIZE);
    if (err != EEPROM_READ_SIZE){
        PRINTFD("Error: read failed %s\n",
                EEPROM_CHIP_NAME);
        unitTestLogFail();
        return err;
    }
    err = close(fd);
    if (err < 0){
        PRINTFD("Error: failed to close %u\n", fd);
        unitTestLogFail();
        return err;
    }
    revision=eepromBuffer[EEPROM_PCB_REVISION_OFFSET];
    // Deallocate memory
    free(eepromBuffer);
    return revision;
}
#endif

rtems_status_code brdInitXlink(void)
{
    XLinkError_t status = X_LINK_COMMUNICATION_NOT_OPEN;

    ghandler.loglevel = 0;
    ghandler.profEnable = 0;

    printf ( "\n\nWaiting for handshake with xlink PC application...\n\n" );
    while( status != X_LINK_SUCCESS) {
        status = XLinkInitialize(&ghandler);
    }
    printf ( "PC connected!\n" );

    return RTEMS_SUCCESSFUL;
}

/// ===  FUNCTION  ======================================================================
///  Name:  brdInit
///  Description: board init top level function
/// =====================================================================================
rtems_status_code brdInit(void)
{
#ifdef POWER_MEASUREMENT
    rtems_status_code ret;
#endif
    /// Init CPR
    brdInitCpr();

    /// Init Gpios
    brdInitGpios();

    /// Init I2C
    brdInitI2c();

    /// Init aux devices
    brdInitAuxDevices();

#ifdef USE_ONE_LEON
    // init mipi Rx/Tx driver, Register all mipi rx an tx controller.
    uint32_t mipi_major;
    int sc;
    struct OsDrvMipiInitDriverCfg driver_config = {
      .irq_priority = 5,
      .loopback_mode = 0,
    };
    sc = rtems_io_register_driver(0, &mipi_drv_tbl, &mipi_major);
    assert(sc == RTEMS_SUCCESSFUL);
    sc = rtems_io_initialize(mipi_major, 0, &driver_config);
    assert(sc == RTEMS_SUCCESSFUL);
#else
#endif
    /// Init Cams
    brdInitCameras();

    initClocksAndMemory();
#ifdef POWER_MEASUREMENT

	ret = UsbInit();
	if (0 != ret) {
		printf("UsbInit failed. ret:%d\n", ret);
	}else{
		printf("UsbInit Succeeded.\n");
	}

#else
	int ret = UsbInit();
	if (0 != ret) {
		printf("UsbInit failed. ret:%d\n", ret);
	}else{
		printf("UsbInit Succeeded.\n");
	}
	return ret;
#endif
#ifdef POWER_MEASUREMENT
    int32_t sc;
//    OsDrvCprPwrIsland unusedPowerIslands[] = {
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_0,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_1,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_2,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_3,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_4,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_5,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_6,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_7,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_8,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_9,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_10,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_11,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_12,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_13,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_14,
//        OS_DRV_CPR_PWR_ISLAND_SHAVE_15,
//        OS_DRV_CPR_PWR_ISLAND_MSS_VENC,
//        OS_DRV_CPR_PWR_ISLAND_PCIE,
//    };
//
//    // Cut the unused power islands
//    for(unsigned int ipi = 0; ipi < (sizeof(unusedPowerIslands)/sizeof(OsDrvCprPwrIsland)); ipi++)
//    {
//       ret = OsDrvCprTurnOffIsland(unusedPowerIslands[ipi], true);
//       if (ret != RTEMS_SUCCESSFUL) {
//               printf("PowerOffIsland  %u failed \n",unusedPowerIslands[ipi]);
//           }
//    }
//    pcbRevision = revisionDetect();
//
//    assert(pcbRevision > 0); // error when reading revision
//
//    // Switch to default configuration if unknown revision
//    if (pcbRevision > MV0235_REV4) {
//        pcbRevision = MV0235_REV4;
//    }
//    // Init drivers for temp and power
//    sc = initPwrDriver(pcbRevision);
//    if (sc != RTEMS_SUCCESSFUL) {
//        printf("initPwrDriver() failed\n");
//    }
    sc = initTempDriver();
    if (sc != RTEMS_SUCCESSFUL) {
        printf("initTemprDriver() failed\n");
    }
#endif

    return RTEMS_SUCCESSFUL;
}
