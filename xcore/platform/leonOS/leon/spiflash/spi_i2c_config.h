/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _SPI_I2C_CONFIG_H
#define _SPI_I2C_CONFIG_H

// 1: Includes
// ----------------------------------------------------------------------------

// Include board specific config functions
// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define L2CACHE_NORMAL_MODE     (0x6)  // In this mode the L2Cache acts as a cache for the DRAM
#define L2CACHE_CFG             (L2CACHE_NORMAL_MODE)
#define BIGENDIANMODE           (0x01000786)

// 3:  Exported Global Data (generally better to avoid)
// ----------------------------------------------------------------------------

// 4:  Exported Functions (non-inline)
// ----------------------------------------------------------------------------
/// Setup all the clock configurations needed by this application and also the ddr
///
/// @return    0 on success, non-zero otherwise
void v_myr2_spi_test_pin_config(void);
int i_myr2_register_libi2c_spi_bus(void);

#endif // _SPI_I2C_CONFIG_H

/* End of File */