/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/


#ifndef LRT_PLAT_COMMON_H
#define LRT_PLAT_COMMON_H

// Includes
#include "mv_types.h"
#include "xeye_info.h"
#include "xeye_BM_timer.h"
#ifdef __cplusplus
extern "C" {
#endif
// Exported Global Data (generally better to avoid)
typedef struct {
   uint32_t sys_clock;
   bool sipp_enable;
   bool camera_enable;
   bool board_init_enable;
   bool sys_clock_enable;
} LrtSysConfig_t;
// Exported Functions (non-inline)

extern bool leonRT_platform_init(const LrtSysConfig_t sys_config, XeyeBoardInfo_t* xeye_board_info);
#ifdef __cplusplus
}
#endif
#endif // LRT_PLAT_COMMON_H
