///
/// @file usb_vsc.h
/// @copyright All code copyright Movidius Ltd 2017, all rights reserved
///            For License Warranty see: common/license.txt
/// @defgroup USB_VSC XLink
/// @{
/// @brief     USB VSC API Functions.
///

#ifndef USB_VSC_H
#define USB_VSC_H

// 1: Includes
// ----------------------------------------------------------------------------
#include "usbpump_application_rtems_api.h"
#include <stddef.h>
#include <rtems.h>

#define USBPUMP_MDK_CACHE_ENABLE 1
#define USB_VSC_DEVNAME "/dev/usbvsc"

#define ALIGNED(x) __attribute__((aligned(x)))

#ifdef __cplusplus
extern "C"
{
#endif

// 2:  Exported Global Data
// ----------------------------------------------------------------------------
typedef enum {
   EMPTY
} VSCControlType;

// 3:  Exported Functions
// ----------------------------------------------------------------------------
extern rtems_driver_address_table VSCCompAdrTbl;

int is_usb_reconnected(void);
int reset_usb_reconnect_flag(void);

#ifdef __cplusplus
}
#endif

#endif

///@}
