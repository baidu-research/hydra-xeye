/* =============================================================================
* Copyright (c) 2013-2015 MM Solutions AD
* All rights reserved. Property of MM Solutions AD.
*
* This source code may not be used against the terms and conditions stipulated
* in the licensing agreement under which it has been supplied, or without the
* written permission of MM Solutions. Rights to use, copy, modify, and
* distribute or disclose this source code and its documentation are granted only
* through signed licensing agreement, provided that this copyright notice
* appears in all copies, modifications, and distributions and subject to the
* following conditions:
* THIS SOURCE CODE AND ACCOMPANYING DOCUMENTATION, IS PROVIDED AS IS, WITHOUT
* WARRANTY OF ANY KIND, EXPRESS OR IMPLIED. MM SOLUTIONS SPECIFICALLY DISCLAIMS
* ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN
* NO EVENT SHALL MM SOLUTIONS BE LIABLE TO ANY PARTY FOR ANY CLAIM, DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
* PROFITS, OR OTHER LIABILITY, ARISING OUT OF THE USE OF OR IN CONNECTION WITH
* THIS SOURCE CODE AND ITS DOCUMENTATION.
* =========================================================================== */
/**
* @file app_guzzi_command_dbg.h
*
* @author ( MM Solutions AD )
*
*/
/* -----------------------------------------------------------------------------
*!
*! Revision History
*! ===================================
*! 03-Jul-2015 : Author ( MM Solutions AD )
*! Created
* =========================================================================== */

#ifndef _APP_GUZZI_COMMAND_DBG_H
#define _APP_GUZZI_COMMAND_DBG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_guzzi_command.h"

int app_guzzi_command_dbg_peek(
        void *app_private,
        app_guzzi_command_callback_t *callback
    );
void app_guzzi_command_dbg_wait(
        void *app_private,
        app_guzzi_command_callback_t *callback
    );
int app_guzzi_command_dbg_wait_timeout(
        void *app_private,
        app_guzzi_command_callback_t *callback,
        uint32_t timeout_ms
    );


#ifdef __cplusplus
}
#endif

#endif /* _APP_GUZZI_COMMAND_DBG_H */
