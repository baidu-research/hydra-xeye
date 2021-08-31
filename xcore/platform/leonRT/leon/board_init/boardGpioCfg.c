/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include <DrvGpio.h>
#include "boardGpioCfg.h"

const drvGpioInitArrayType brdGpioCfgDefault = {
    // -----------------------------------------------------------------------
    // SD Configuration
    // -----------------------------------------------------------------------
    {
        16, 16 , ACTION_UPDATE_ALL                 // SDIO interface
        ,
        0
        ,
        D_GPIO_MODE_3                              // SDIO Mode
        ,
        D_GPIO_PAD_DRIVE_4mA | D_GPIO_PAD_RECEIVER_ON
        , NULL
    },

    {
        17, 17 , ACTION_UPDATE_ALL                 // SDIO interface
        ,
        0
        ,
        D_GPIO_MODE_3                              // SDIO Mode
        ,
        D_GPIO_PAD_DRIVE_4mA
        , NULL
    },

    {
        18, 21 , ACTION_UPDATE_ALL                 // SDIO interface
        ,
        0
        ,
        D_GPIO_MODE_3                              // SDIO Mode
        ,
        D_GPIO_PAD_DRIVE_4mA | D_GPIO_PAD_RECEIVER_ON
        , NULL
    },

    // -----------------------------------------------------------------------
    // InfraRed Sensor Pin
    // -----------------------------------------------------------------------

    {
        14, 14 , ACTION_UPDATE_ALL           // Infrared Sensor Pin
        ,
        PIN_LEVEL_LOW               //
        ,
        D_GPIO_MODE_7            |  // Direct GPIO mode
        D_GPIO_DIR_IN            |  // Input
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },

    // -----------------------------------------------------------------------
    // I2C0 Configuration  (General use)
    // -----------------------------------------------------------------------
    {
        60, 60, ACTION_UPDATE_ALL          // I2C0 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select I2C Mode
        D_GPIO_DIR_OUT            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c0_scl"
    },
    {
        61, 61, ACTION_UPDATE_ALL          // I2C0 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select I2C Mode
        D_GPIO_DIR_OUT            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c0_sda"
    },
    // -----------------------------------------------------------------------
    // I2C1 Configuration  (AP Uplink)
    // -----------------------------------------------------------------------
    {
        12, 12,
        ACTION_UPDATE_ALL          // I2C1 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_2            |
        D_GPIO_DIR_OUT            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c1_scl"
    },
    {
        13, 13,
        ACTION_UPDATE_ALL          // I2C1 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_2            |
        D_GPIO_DIR_OUT            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c1_sda"
    },
    // -----------------------------------------------------------------------
    // I2C2 Configuration  (General Use)
    // -----------------------------------------------------------------------
    {
        79, 79,
        ACTION_UPDATE_ALL          // I2C2 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_2            |
        D_GPIO_DIR_OUT           |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c2_scl"
    },
    {
        80, 80,
        ACTION_UPDATE_ALL          // I2C2 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_2            |
        D_GPIO_DIR_OUT           |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c2_sda"
    },
#ifdef SPI_SLAVE
    // -----------------------------------------------------------------------
    // SPI0 Slave Configuration (Boot SPI/AP Uplink)
    // -----------------------------------------------------------------------
    {
        74, 74, ACTION_UPDATE_ALL          //
        ,
        PIN_LEVEL_LOW               //
        ,
        D_GPIO_MODE_0            |  //
        D_GPIO_DIR_IN            |  // Drive in
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },

    {
        75, 75, ACTION_UPDATE_ALL          //
        ,
        PIN_LEVEL_LOW               //
        ,
        D_GPIO_MODE_0            |  //
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    {
        76, 77, ACTION_UPDATE_ALL          //
        ,
        PIN_LEVEL_HIGH              //
        ,
        D_GPIO_MODE_0            |  //
        D_GPIO_DIR_IN            |  // Drive in
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
#else
    // SPI0 Configuration (Boot SPI/AP Uplink)
    // -----------------------------------------------------------------------
    {
        74, 77, ACTION_UPDATE_ALL          //
        ,
        PIN_LEVEL_LOW               //
        ,
        D_GPIO_MODE_0            |  //
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
#endif

    // -----------------------------------------------------------------------
    // LCD Configuration
    //
    // -----------------------------------------------------------------------
    {
        30, 30 , ACTION_UPDATE_ALL          // LCD_VSYNC
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select LCD
        D_GPIO_DIR_OUT           |  // WARNING: NOT PER THE ORGINAL
        D_GPIO_DATA_INV_ON
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    {
        29, 29 , ACTION_UPDATE_ALL          // LCD_PCLK
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select LCD  (Note: It will be reconfigured by ext PLL driver)
        D_GPIO_DIR_IN |
        D_GPIO_DATA_INV_ON
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    {
        31, 31 , ACTION_UPDATE_ALL          // LCD_HSYNC
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select LCD
        D_GPIO_DIR_OUT           |  // WARNING: NOT PER THE ORGINAL
        D_GPIO_DATA_INV_ON
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    {
        32, 32 , ACTION_UPDATE_ALL
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |
        D_GPIO_DIR_IN
        ,
        D_GPIO_PAD_DEFAULTS
        , "bmi160_int1"
    },
    {
        34, 34 , ACTION_UPDATE_ALL
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |
        D_GPIO_DIR_IN
        ,
        D_GPIO_PAD_DEFAULTS
        , "bmi160_int2"
    },
    {
        36, 45 , ACTION_UPDATE_ALL          // LCD D0->D9
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select LCD
        D_GPIO_DIR_OUT            | // WARNING: NOT PER THE ORGINAL
        D_GPIO_DATA_INV_OFF
        ,
        D_GPIO_PAD_DEFAULTS      |  // Uses the default PAD configuration
        D_GPIO_PAD_SLEW_FAST        // But with fast slew rate
        , NULL
    },
    {
        82, 84 , ACTION_UPDATE_ALL          // LCD D10 -> D12
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_3            |  // Mode 3 to select LCD
        D_GPIO_DIR_OUT            |  // WARNING: NOT PER THE ORGINAL
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF//        |              D_GPIO_IRQ_SRC_NONE
        ,
        D_GPIO_PAD_DEFAULTS      |  // Uses the default PAD configuration
        D_GPIO_PAD_SLEW_FAST        // But with fast slew rate
        , NULL
    },
    {
        49, 51 , ACTION_UPDATE_ALL          // LCD D13-> D15
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select LCD
        D_GPIO_DIR_OUT            | // WARNING: NOT PER THE ORGINAL
        D_GPIO_DATA_INV_OFF
        ,
        D_GPIO_PAD_DEFAULTS      |  // Uses the default PAD configuration
        D_GPIO_PAD_SLEW_FAST        // But with fast slew rate
        , NULL
    },
    // cdcel ENABLE
    {
        58, 58 , ACTION_UPDATE_ALL           // cdcel ENABLE
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },

    {
        15, 15 , ACTION_UPDATE_ALL           // cdcel ENABLE
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "ov7750_bus0_reset_right"
    },
    {
        33, 33 , ACTION_UPDATE_ALL           // cdcel ENABLE
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "ov7750_bus0_reset_left"
    },

    // -----------------------------------------------------------------------
    // Finally we terminate the Array
    // -----------------------------------------------------------------------
    {
        0, 0   , ACTION_TERMINATE_ARRAY     // Do nothing but simply termintate the Array
        ,
        PIN_LEVEL_LOW               // Won't actually be updated
        ,
        D_GPIO_MODE_0               // Won't actually be updated
        ,
        D_GPIO_PAD_DEFAULTS         // Won't actually be updated
        , NULL
    }
};


const drvGpioInitArrayType brdXeye2GpioCfgDefault = {
    // -----------------------------------------------------------------------
    // UART Pin Configuration
    // -----------------------------------------------------------------------
    {
        31, 31 , ACTION_UPDATE_ALL
        ,
        0
        ,
        D_GPIO_MODE_4            |                 // uart_apb_sout Mode
        D_GPIO_DIR_OUT            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS
        , NULL
    },

    {
        33, 33 , ACTION_UPDATE_ALL
        ,
        0
        ,
        D_GPIO_MODE_4            |                 // uart_apb_sin Mode
        D_GPIO_DIR_IN            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS | D_GPIO_PAD_PULL_UP
        , NULL
    },
    // -----------------------------------------------------------------------
    // ToF Trigger Pin Configuration
    // -----------------------------------------------------------------------
    {
        55, 55 , ACTION_UPDATE_ALL                 // Trigger IN interface
        ,
        0
        ,
        D_GPIO_MODE_7            |                 // CPU Drect Control Mode
        D_GPIO_DIR_IN            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS
        , NULL
    },

    // -----------------------------------------------------------------------
    // SD Configuration
    // -----------------------------------------------------------------------
    {
        16, 16 , ACTION_UPDATE_ALL                 // SDIO interface
        ,
        0
        ,
        D_GPIO_MODE_3                              // SDIO Mode: SD_HST1_DAT_3
        ,
        D_GPIO_PAD_DRIVE_4mA | D_GPIO_PAD_RECEIVER_ON
        , NULL
    },

    {
        17, 17 , ACTION_UPDATE_ALL                 // SDIO interface
        ,
        0
        ,
        D_GPIO_MODE_3                              // SDIO Mode: SD_HST1_DAT_CLK
        ,
        D_GPIO_PAD_DRIVE_4mA
        , NULL
    },

    {
        18, 21 , ACTION_UPDATE_ALL                 // SDIO interface
        ,
        0
        ,
        D_GPIO_MODE_3                              // SDIO Mode: SD_HST1_DAT_CMD、SD_HST1_DAT_DAT_0、SD_HST1_DAT_DAT_1、SD_HST1_DAT_DAT_2
        ,
        D_GPIO_PAD_DRIVE_4mA | D_GPIO_PAD_RECEIVER_ON
        , NULL
    },
    // -----------------------------------------------------------------------
    // I2C0 Configuration  (General use)
    // -----------------------------------------------------------------------
    {
        60, 60, ACTION_UPDATE_ALL          // I2C0 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select I2C Mode
        D_GPIO_DIR_OUT            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c0_scl"
    },
    {
        61, 61, ACTION_UPDATE_ALL          // I2C0 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select I2C Mode
        D_GPIO_DIR_OUT            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c0_sda"
    },
    // -----------------------------------------------------------------------
    // I2C1 Configuration  (AP Uplink)
    // -----------------------------------------------------------------------
    {
        12, 12, ACTION_UPDATE_ALL          // I2C1 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_2            |
        D_GPIO_DIR_OUT            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c1_scl"
    },
    {
        13, 13, ACTION_UPDATE_ALL          // I2C1 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_2            |
        D_GPIO_DIR_OUT            |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c1_sda"
    },
    // -----------------------------------------------------------------------
    // I2C2 Configuration  (General Use)
    // -----------------------------------------------------------------------
    {
        79, 79, ACTION_UPDATE_ALL          // I2C2 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_2            |
        D_GPIO_DIR_OUT           |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c2_scl"
    },
    {
        80, 80, ACTION_UPDATE_ALL          // I2C2 Pins
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_2            |
        D_GPIO_DIR_OUT           |
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "i2c2_sda"
    },
#ifdef SPI_SLAVE
    // -----------------------------------------------------------------------
    // SPI0 Slave Configuration (Boot SPI/AP Uplink)
    // -----------------------------------------------------------------------
    {
        74, 74, ACTION_UPDATE_ALL          //
        ,
        PIN_LEVEL_LOW               //
        ,
        D_GPIO_MODE_0            |  //
        D_GPIO_DIR_IN            |  // Drive in
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },

    {
        75, 75, ACTION_UPDATE_ALL          //
        ,
        PIN_LEVEL_LOW               //
        ,
        D_GPIO_MODE_0            |  //
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    {
        76, 77, ACTION_UPDATE_ALL          //
        ,
        PIN_LEVEL_HIGH              //
        ,
        D_GPIO_MODE_0            |  //
        D_GPIO_DIR_IN            |  // Drive in
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
#else
    // SPI0 Configuration (Boot SPI/AP Uplink)
    // -----------------------------------------------------------------------
    {
        74, 77, ACTION_UPDATE_ALL          //
        ,
        PIN_LEVEL_LOW               //
        ,
        D_GPIO_MODE_0            |  //
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
#endif
    // -----------------------------------------------------------------------
    // LCD Configuration
    // -----------------------------------------------------------------------
    {
        30, 30 , ACTION_UPDATE_ALL          // LCD_VSYNC
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select LCD
        D_GPIO_DIR_OUT           |  // WARNING: NOT PER THE ORGINAL
        D_GPIO_DATA_INV_ON
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    {
        29, 29 , ACTION_UPDATE_ALL          // LCD_PCLK
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select LCD  (Note: It will be reconfigured by ext PLL driver)
        D_GPIO_DIR_IN |
        D_GPIO_DATA_INV_ON
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    /*{31,31 , ACTION_UPDATE_ALL           // LCD_HSYNC
     ,
     PIN_LEVEL_LOW               // Default to driving low
     ,
     D_GPIO_MODE_0            |  // Mode 0 to select LCD
     D_GPIO_DIR_OUT           |  // WARNING: NOT PER THE ORGINAL
     D_GPIO_DATA_INV_ON
     ,
     D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
     , NULL
    },*/
    {
        32, 32 , ACTION_UPDATE_ALL          // LCD_DATAEN
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select LCD
        D_GPIO_DIR_OUT
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    {
        36, 45 , ACTION_UPDATE_ALL          // LCD D0->D9
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select LCD
        D_GPIO_DIR_OUT           | // WARNING: NOT PER THE ORGINAL
        D_GPIO_DATA_INV_OFF
        ,
        D_GPIO_PAD_DEFAULTS      |  // Uses the default PAD configuration
        D_GPIO_PAD_SLEW_FAST        // But with fast slew rate
        , NULL
    },
    {
        82, 84 , ACTION_UPDATE_ALL          // LCD D10 -> D12
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_3            |  // Mode 3 to select LCD
        D_GPIO_DIR_OUT           |  // WARNING: NOT PER THE ORGINAL
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF//        |              D_GPIO_IRQ_SRC_NONE
        ,
        D_GPIO_PAD_DEFAULTS      |  // Uses the default PAD configuration
        D_GPIO_PAD_SLEW_FAST        // But with fast slew rate
        , NULL
    },
    {
        49, 51 , ACTION_UPDATE_ALL          // LCD D13-> D15
        ,
        PIN_LEVEL_LOW               // Default to driving low
        ,
        D_GPIO_MODE_0            |  // Mode 0 to select LCD
        D_GPIO_DIR_OUT            | // WARNING: NOT PER THE ORGINAL
        D_GPIO_DATA_INV_OFF
        ,
        D_GPIO_PAD_DEFAULTS      |  // Uses the default PAD configuration
        D_GPIO_PAD_SLEW_FAST        // But with fast slew rate
        , NULL
    },
    // -----------------------------------------------------------------------
    // CDCEL Select Pin
    // -----------------------------------------------------------------------
    {
        58, 58 , ACTION_UPDATE_ALL  // cdcel ENABLE
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    // -----------------------------------------------------------------------
    // InfraRed Sensor (useless)
    // -----------------------------------------------------------------------
    {
        14, 14 , ACTION_UPDATE_ALL           // Infrared Sensor Pin
        ,
        PIN_LEVEL_LOW               //
        ,
        D_GPIO_MODE_7            |  // Direct GPIO mode
        D_GPIO_DIR_IN            |  // Input
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    // -----------------------------------------------------------------------
    // CAM_A_GPIO0 (useless)
    // -----------------------------------------------------------------------
    {
        59, 59 , ACTION_UPDATE_ALL  // CAM_A_GPIO0 Pin
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_IN            |  // Input
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "CAM_A_GPIO0"
    },
    // -----------------------------------------------------------------------
    // CAM_B_GPIO0 (useless)
    // -----------------------------------------------------------------------
    {
        15, 15 , ACTION_UPDATE_ALL  // CAM_B_GPIO0 Pin
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "CAM_B_GPIO0"
    },
    // -----------------------------------------------------------------------
    // CAM_A_PWM(debug LED)
    // -----------------------------------------------------------------------
    {
        27, 27 , ACTION_UPDATE_ALL  // CAM_A_PWM Pin
        ,
        PIN_LEVEL_HIGH
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , "CAM_A_PWM"
    },
    // -----------------------------------------------------------------------
    // CAM_B_PWM
    // -----------------------------------------------------------------------
    /*{33, 33, ACTION_UPDATE_ALL   // CAM_B_PWM Pin
     ,
     PIN_LEVEL_LOW
     ,
     D_GPIO_MODE_7            |  // Mode 7 to select GPIO
     D_GPIO_DIR_OUT           |  // Drive out
     D_GPIO_DATA_INV_OFF      |
     D_GPIO_WAKEUP_OFF
     ,
     D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
     , "CAM_B_PWM"
    },*/
    // -----------------------------------------------------------------------
    // COM_IO1 => COM_IO1_ISO (Debug GPIO TP6)
    // -----------------------------------------------------------------------
    {
        52, 52, ACTION_UPDATE_ALL    // COM_IO1 Pin
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_IN            |  // Input
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    // -----------------------------------------------------------------------
    // COM_IO2 => COM_IO2_ISO (Debug GPIO TP1)
    // -----------------------------------------------------------------------
    {
        53, 53, ACTION_UPDATE_ALL    // COM_IO2 Pin
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_IN            |  // Input
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    // -----------------------------------------------------------------------
    // COM_IO3 => GPIO_OUT1 (Debug GPIO TP1(Up board))
    // -----------------------------------------------------------------------
    {
        54, 54, ACTION_UPDATE_ALL    // COM_IO3 Pin
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    // -----------------------------------------------------------------------
    // COM_IO4 => GPIO_IN1 (Debug GPIO TP2(Up board))
    // -----------------------------------------------------------------------
    {
        55, 55, ACTION_UPDATE_ALL    // COM_IO4 Pin
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_OUT           |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },
    // -----------------------------------------------------------------------
    // COM_IO5 => COM_IO5_ISO (Debug GPIO TP5)
    // -----------------------------------------------------------------------
    {
        56, 56, ACTION_UPDATE_ALL    // COM_IO5 Pin
        ,
        PIN_LEVEL_LOW
        ,
        D_GPIO_MODE_7            |  // Mode 7 to select GPIO
        D_GPIO_DIR_IN            |  // Drive out
        D_GPIO_DATA_INV_OFF      |
        D_GPIO_WAKEUP_OFF
        ,
        D_GPIO_PAD_DEFAULTS         // Uses the default PAD configuration
        , NULL
    },

    // -----------------------------------------------------------------------
    // Finally we terminate the Array
    // -----------------------------------------------------------------------
    {
        0, 0   , ACTION_TERMINATE_ARRAY     // Do nothing but simply termintate the Array
        ,
        PIN_LEVEL_LOW               // Won't actually be updated
        ,
        D_GPIO_MODE_0               // Won't actually be updated
        ,
        D_GPIO_PAD_DEFAULTS         // Won't actually be updated
        , NULL
    }
};
