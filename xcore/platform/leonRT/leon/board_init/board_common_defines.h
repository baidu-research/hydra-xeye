/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BOARD_COMMON_DEFINES_H_
#define BOARD_COMMON_DEFINES_H_

#include "DrvGpioDefines.h"

// 1: Defines
// ----------------------------------------------------------------------------

#define NUM_CHARS_BRD_NAME 6

// 2: Typedefs (types, enums, structs)
// ----------------------------------------------------------------------------

typedef drvGpioConfigRangeType BoardGPIOConfig;

typedef struct {
    uint32_t version;
    const BoardGPIOConfig* gpioConfig;
} MV0212GpioCfg;

typedef enum  {
    BRDCONFIG_SUCCESS            =  0,
    BRDCONFIG_ERROR              = -1,
    BRDCONFIG_I2C_SLAVE_ERROR    = -2,
    BRDCONFIG_I2C_DRIVER_ERROR   = -3,
    BRDCONFIG_INVALIDBRD         = -4
} BoardErrorCode;

typedef enum  {
    BRDCONFIG_END = 0,    /*< Configuration terminator */
    BRDCONFIG_GPIO,       /*< GPIO configuration */
    BRDCONFIG_I2C,        /*< I2C Configuration */
    BRDCONFIG_I2CDEVLIST  /*< List of I2C devices to be found */
} BoardConfigType;

typedef struct {
    uint32_t revision;
    char     name[NUM_CHARS_BRD_NAME + 1];
} BoardInfo;

typedef struct {
    BoardConfigType type;
    void* config;
} BoardConfigDesc;

#endif
