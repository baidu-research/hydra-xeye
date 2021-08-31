///
/// @file      boardconfig.h
/// @copyright All code copyright Movidius Ltd 2017, all rights reserved.
///                  For License Warranty see: common/license.txt
/// @brief     Board configuration header.
///
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

/// System Includes
/// -------------------------------------------------------------------------------------
#include <rtems/status-checks.h>

/// Application Includes
/// -------------------------------------------------------------------------------------

/// Source Specific #defines and types (typedef,enum,struct)
/// -------------------------------------------------------------------------------------
#ifndef PLL_DESIRED_FREQ_KHZ
#define PLL_DESIRED_FREQ_KHZ        (600000) /// PLL desired frequency
#endif

#ifndef DEFAULT_OSC0_KHZ
#define DEFAULT_OSC0_KHZ            (24000)  /// Input OSC frequency
#endif

/// Exported global Data (Only if absolutely necessary)
/// -------------------------------------------------------------------------------------

/// Exported function headers
/// -------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
void in_symbol_process(uint16_t in_symbol);

rtems_status_code brdInit(void);
rtems_status_code brdInitCameras(void);
rtems_status_code brdInitAuxDevices(void);
#ifdef POWER_MEASUREMENT
///Read temperature
rtems_status_code readTemp(void);
///Read power and current consumption
rtems_status_code readPower(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // _BOARD_CONFIG_H_
