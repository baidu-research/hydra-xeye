/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _BOARD_INIT_BM_H_
#define _BOARD_INIT_BM_H_

#include "board_defines.h"
#include "platform_pubdef.h"

#define BOARD_212_NAME          "MV0212"
#define NUM_I2C_DEVS            3

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Configures the External PLL to a given frequency
///
/// @param[in] clock_config (See "DrvCDCEL.h" for usable indexes)
/// @return
///         - BRDCONFIG_SUCCESS
///         - non-zero on fail
///
int32_t xeyeInitExtPll(uint32_t clock_config);

/// @brief Initialise the default configuration for I2C0, I2C1, I2C2 on the MV0212 Board
/// and return the device information
///
/// @param[out] info        pointer to the I2C device configuration strucure
/// @param[in]  interfaces  minimum number of I2C interfaces
/// @return
///         - BRDCONFIG_SUCCESS
///         - BRDCONFIG_ERROR
///
int32_t xeyeGetI2CInfo(BoardI2CInfo* info, uint32_t interfaces);
/// @brief Returns the revision number of the PCB
///
/// @param[out] brd_rev Board revision
/// @return
///         - BRDCONFIG_SUCCESS
///         - BRDCONFIG_ERROR
///         - BRDCONFIG_I2C_SLAVE_ERROR
///         - BRDCONFIG_I2C_DRIVER_ERROR
///
int32_t XeyeGetPCBRevision(uint32_t* brd_rev);

/// @brief This function initializes the basic functions of MV0212 board: I2C buses and sets up GPIOs
///
/// @param[in] init_cfg - The board configuration structure
/// @return
///         - BRDCONFIG_SUCCESS
///         - BRDCONFIG_ERROR
///         - BRDCONFIG_I2C_SLAVE_ERROR
///         - BRDCONFIG_I2C_DRIVER_ERROR
///
int32_t XeyeBoardInit(BoardConfigDesc* init_cfg);
int32_t xeyeGetBrdInfo(BoardInfo* info);
int32_t xeyeIneterfaceInit(XeyePcbType pcb_version);
#ifdef __cplusplus
}
#endif

#endif
