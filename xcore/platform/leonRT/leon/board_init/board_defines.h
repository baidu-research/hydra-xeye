/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/


#ifndef BOARD_DEFINES_H_
#define BOARD_DEFINES_H_

#include "board_common_defines.h"
#include "DrvI2cMasterDefines.h"

// 1: Defines
// ----------------------------------------------------------------------------

#define MV0212_OSC_IN_FREQ_KHZ         12000

// ----------------------------------------------------------------------------
// I2C
// ----------------------------------------------------------------------------

// I2C0 -- Master
#define MV0212_I2C0_SCL_PIN            (60)
#define MV0212_I2C0_SDA_PIN            (61)
#define MV0212_I2C0_SPEED_KHZ_DEFAULT  (100)
#define MV0212_I2C0_ADDR_SIZE_DEFAULT  (ADDR_7BIT)

// I2C1 -- Master
#define MV0212_I2C1_SCL_PIN            (12)
#define MV0212_I2C1_SDA_PIN            (13)
#define MV0212_I2C1_SPEED_KHZ_DEFAULT  (100)
#define MV0212_I2C1_ADDR_SIZE_DEFAULT  (ADDR_7BIT)

// I2C2 -- Master
#define MV0212_I2C2_SCL_PIN            (79)
#define MV0212_I2C2_SDA_PIN            (80)
#define MV0212_I2C2_SPEED_KHZ_DEFAULT  (100)
#define MV0212_I2C2_ADDR_SIZE_DEFAULT  (ADDR_7BIT)

// ----------------------------------------------------------------------------
// General GPIO definition
// ----------------------------------------------------------------------------
#define MV0212_WM8325_I2C_ADDR_7BIT    (0x36)

// GPIO:  Inputs

// GPIO:  Outputs
#define MV0212_PIN_CAM_A_GPIO0_N           (59)
#define MV0212_PIN_CAM_B_GPIO0_N           (15)
#define MV0212_PIN_COM_IO5_N               (56)
#define MV0212_PIN_AP_IRQ                  (22)

// PCB Revision Detection
#define MV0212_REV_DETECT  (9)

// predefined MIPI media devices for mv212 board
#define CAM_A1_MIPICTRL   MIPI_CTRL_0
#define CAM_B1_MIPICTRL   MIPI_CTRL_2
#define CAM_B2_MIPICTRL   MIPI_CTRL_3

// predefined I2C addressing slots, for paired cameras on daughterboard - mv212
#define CAM_B1_RIGHT_ADDR 0
#define CAM_B2_LEFT_ADDR  1
#define CAM_A_ADDR        0

// predefined camera sensors reset pins for every couple mv212 board - sensor daughter board
#define XEYE_MIPI0_RST_GPIO         MV0212_PIN_CAM_A_GPIO0_N
#define XEYE_MIPI1_RST_GPIO         MV0212_PIN_CAM_B_GPIO0_N

// predefined GPIO's used for camera VSYNC/HSYNC generation on mv212 board
#define CAMERA_VSYNC_GPIO        0xFF  //none, not allowed
#define CAMERA_HSYNC_GPIO        0xFF  //none, not allowed
#define CAMERA_VSYNC_GPIO_MODE   0xFF  //none, not allowed
#define CAMERA_HSYNC_GPIO_MODE   0xFF  //none, not allowed

// 2: Typedefs (types, enums, structs)
// ----------------------------------------------------------------------------
typedef tyI2cConfig BoardI2CConfig;

typedef struct {
    uint32_t version;
    BoardI2CConfig** i2cCfg; /* Must be a NULL terminated list */
} XEYEI2CCfg;

typedef struct {
    I2CM_Device* handler;
} BoardI2CInfo;

#endif
