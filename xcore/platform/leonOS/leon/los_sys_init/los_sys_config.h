/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef LOS_SYS_CONFIG_H
#define LOS_SYS_CONFIG_H
#ifdef  __cplusplus
extern "C" {
#endif
// 1: Includes
#include "mv_types.h"

// 2:  Source Specific #defines and types  (typedef,enum,struct)

typedef enum {
    DEFAULT_MEM_POLICY = 0,
    XEYE_APP_MEM_POLICY,
    HUATU_FACE_MEM_POLICY,
    YOUBAO_FACE_MEM_POLICY,
    SHIMU_MEM_POLICY,
    CROWD_MEM_POLICY,
    NEW_RETAIL_MEM_POLICY
} mv_mem_policy_t;

// 3:  Exported Global Data (generally better to avoid)

// 4:  Exported Functions (non-inline)
extern int initClocks(uint32_t sys_clock);
extern int initMemory(mv_mem_policy_t mem_policy);
extern int XeyeClockControl(uint8_t usb_disable, uint8_t sdio_disable);
#ifdef  __cplusplus
}
#endif
#endif // LOS_SYS_CONFIG_H
