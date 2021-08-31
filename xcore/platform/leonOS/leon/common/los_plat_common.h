/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef LOS_PLAT_COMMON_H
#define LOS_PLAT_COMMON_H

// Includes
#include "mv_types.h"
#include "los_sys_config.h"
#include "xeye_rtems_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Exported Global Data (generally better to avoid)
typedef struct {
   bool usb_enable;
   bool temperature_enable;
   bool network_enable;
   bool storage_enable;
   bool watchdog_enable;
   bool uart_enable;
   bool sys_clock_enable;
   bool sys_memory_enable;
   mv_mem_policy_t mem_policy;
   uint32_t sys_clock_Hz;
} LosSysConfig_t;
// Exported Functions (non-inline)
extern bool leonOS_platform_init(LosSysConfig_t sys_config);

#ifdef __cplusplus
}
#endif
#endif // LOS_PLAT_COMMON_H
