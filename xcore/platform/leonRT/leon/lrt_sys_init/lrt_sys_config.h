/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/
 
#ifndef LRT_SYS_CONFIG_H
#define LRT_SYS_CONFIG_H

// Includes
#include "mv_types.h"

#ifdef __cplusplus
extern "C" {
#endif
// Exported Global Data (generally better to avoid)

// Exported Functions (non-inline)

extern int init_lrt_sys_clk(void);
#ifdef __cplusplus
}
#endif
#endif // LRT_SYS_CONFIG_H
