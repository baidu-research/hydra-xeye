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
* @file app_guzzi_command_dbg.c
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
#include <stdlib.h>
#include "app_guzzi_command_dbg.h"

#include <osal/osal_time.h>
#include <swcLeonUtils.h>
#include "app_guzzi_command.h"

#ifndef APP_GUZZI_COMMAND_DBG_PROBE_INTERVAL_MS
#define APP_GUZZI_COMMAND_DBG_PROBE_INTERVAL_MS 10
#endif

uint32_t __attribute__((section(".cmx_direct.data"))) app_guzzi_command_dbg_id = (uint32_t)APP_GUZZI_COMMAND__NOP;
uint32_t __attribute__((section(".cmx_direct.data"))) app_guzzi_command_dbg_cam_id = 0;
uint32_t app_guzzi_command_dbg_cam_lens_move_pos = 0;
uint32_t app_guzzi_command_dbg_cam_ae_manual_exp_us = 0;
uint32_t app_guzzi_command_dbg_cam_ae_manual_sensitivity_iso = 0;
uint32_t app_guzzi_command_dbg_cam_ae_manual_frame_duration_us = 0;
uint32_t app_guzzi_command_dbg_cam_awb_mode = 0;
uint32_t app_guzzi_command_dbg_cam_antibanding_mode = 0;
uint32_t app_guzzi_command_dbg_cam_ae_lock_control = 0;
uint32_t app_guzzi_command_dbg_cam_ae_target_fps_min = 0;
uint32_t app_guzzi_command_dbg_cam_ae_target_fps_max = 0;
uint32_t app_guzzi_command_dbg_cam_awb_lock_control = 0;
uint32_t app_guzzi_command_dbg_cam_capture_intent = 0;
uint32_t app_guzzi_command_dbg_cam_control = 0;
uint32_t app_guzzi_command_dbg_cam_exp_compensation = 0;
uint32_t app_guzzi_command_dbg_cam_iso = 0;
uint32_t app_guzzi_command_dbg_cam_effect_mode = 0;
uint64_t app_guzzi_command_dbg_cam_frame_duration = 0;

#include <utils/mms_debug.h>

mmsdbg_define_variable(
        vdl_app_guzzi_command_dbg,
        DL_DEFAULT,
        0,
        "vdl_app_guzzi_command_dbg",
        "DBG interface for GUZZI App."
    );
#define MMSDEBUGLEVEL mmsdbg_use_variable(vdl_app_guzzi_command_dbg)

#define U(V) swcLeonReadNoCacheU32(uncached((uint32_t)&(V)))
static inline uint32_t uncached(uint32_t v)
{
    if ((0x70000000 <= v) && (v <= 0x70280000)) {
        return v | 0x80000000;
    } else if ((0x80000000 <= v) && (v <= 0xA0000000)) {
        return v | 0x40000000;
    }

    return v;
}

static app_guzzi_command_t *app_guzzi_command_dbg_fill(
        app_guzzi_command_t *command
    )
{
    switch ((app_guzzi_command_id_t)U(app_guzzi_command_dbg_id)) {
        case APP_GUZZI_COMMAND__CAM_START:
            command->id = APP_GUZZI_COMMAND__CAM_START;
            break;
        case APP_GUZZI_COMMAND__CAM_STOP:
            command->id = APP_GUZZI_COMMAND__CAM_STOP;
            break;
        case APP_GUZZI_COMMAND__CAM_CAPTURE:
            command->id = APP_GUZZI_COMMAND__CAM_CAPTURE;
            break;
        case APP_GUZZI_COMMAND__CAM_LENS_MOVE:
            command->id = APP_GUZZI_COMMAND__CAM_LENS_MOVE;
            command->cam.lens_move.pos =
                U(app_guzzi_command_dbg_cam_lens_move_pos);
            break;
        case APP_GUZZI_COMMAND__CAM_AF_TRIGGER:
            command->id = APP_GUZZI_COMMAND__CAM_AF_TRIGGER;
            break;
        case APP_GUZZI_COMMAND__CAM_AE_MANUAL:
            command->id = APP_GUZZI_COMMAND__CAM_AE_MANUAL;
            command->cam.ae_manual.exp_us =
                U(app_guzzi_command_dbg_cam_ae_manual_exp_us);
            command->cam.ae_manual.sensitivity_iso =
                U(app_guzzi_command_dbg_cam_ae_manual_sensitivity_iso);
            command->cam.ae_manual.frame_duration_us =
                U(app_guzzi_command_dbg_cam_ae_manual_frame_duration_us);
            break;
        case APP_GUZZI_COMMAND__CAM_AE_AUTO:
            command->id = APP_GUZZI_COMMAND__CAM_AE_AUTO;
            break;
        case APP_GUZZI_COMMAND__CAM_AWB_MODE:
            command->id = APP_GUZZI_COMMAND__CAM_AWB_MODE;
            command->cam.awb_mode.mode =
                    U(app_guzzi_command_dbg_cam_awb_mode);
            break;
        case APP_GUZZI_COMMAND__CAM_ANTIBANDING_MODE:
            command->id = APP_GUZZI_COMMAND__CAM_ANTIBANDING_MODE;
            command->cam.antibanding_mode.type =
                    U(app_guzzi_command_dbg_cam_antibanding_mode);
            break;
        case APP_GUZZI_COMMAND__CAM_AE_LOCK:
            command->id = APP_GUZZI_COMMAND__CAM_AE_LOCK;
            command->cam.ae_lock_mode.type =
                    U(app_guzzi_command_dbg_cam_ae_lock_control);
            break;
        case APP_GUZZI_COMMAND__CAM_AE_TARGET_FPS_RANGE:
            command->id = APP_GUZZI_COMMAND__CAM_AE_TARGET_FPS_RANGE;
            command->cam.ae_target_fps_range.min_fps =
                    U(app_guzzi_command_dbg_cam_ae_target_fps_min);
            command->cam.ae_target_fps_range.max_fps =
                    U(app_guzzi_command_dbg_cam_ae_target_fps_max);
            break;
        case APP_GUZZI_COMMAND__CAM_AWB_LOCK:
            command->id = APP_GUZZI_COMMAND__CAM_AWB_LOCK;
            command->cam.awb_lock_control.type =
                    U(app_guzzi_command_dbg_cam_awb_lock_control);
            break;
        case APP_GUZZI_COMMAND__CAM_CAPTURE_INTRENT:
            command->id = APP_GUZZI_COMMAND__CAM_CAPTURE_INTRENT;
            command->cam.capture_intent.mode =
                    U(app_guzzi_command_dbg_cam_capture_intent);
            break;
        case APP_GUZZI_COMMAND__CAM_CONTROL_MODE:
            command->id = APP_GUZZI_COMMAND__CAM_CONTROL_MODE;
            command->cam.control_mode.type =
                    U(app_guzzi_command_dbg_cam_control);
            break;
        case APP_GUZZI_COMMAND__CAM_FRAME_DURATION:
            command->id = APP_GUZZI_COMMAND__CAM_FRAME_DURATION;
            command->cam.frame_duration.val =
                    U(app_guzzi_command_dbg_cam_control);
            break;
        case APP_GUZZI_COMMAND__CAM_AE_EXPOSURE_COMPENSATION:
            command->id = APP_GUZZI_COMMAND__CAM_AE_EXPOSURE_COMPENSATION;
            command->cam.exposure_compensation.val =
                    U(app_guzzi_command_dbg_cam_exp_compensation);
            break;
        case APP_GUZZI_COMMAND__CAM_SENSITIVITY:
            command->id = APP_GUZZI_COMMAND__CAM_SENSITIVITY;
            command->cam.sensitivity.iso_val =
                    U(app_guzzi_command_dbg_cam_iso);
            break;
        case APP_GUZZI_COMMAND__CAM_EFFECT_MODE:
            command->id = APP_GUZZI_COMMAND__CAM_EFFECT_MODE;
            command->cam.effect_mode.type =
                    U(app_guzzi_command_dbg_cam_effect_mode);
            break;
        default:
            mmsdbg(
                    DL_ERROR,
                    "Unknown APP GUZZI DBG command id: %d",
                    U(app_guzzi_command_dbg_id)
                );
            command->id = APP_GUZZI_COMMAND__NOP;
    }
    command->cam.id = U(app_guzzi_command_dbg_cam_id);

    return command;
}

int app_guzzi_command_dbg_peek(
        void *app_private,
        app_guzzi_command_callback_t *callback
    )
{
    app_guzzi_command_t command;
    //mmsdbg(DL_ERROR, "command:%d\n", U(app_guzzi_command_dbg_id));
    if (U(app_guzzi_command_dbg_id) != (uint32_t)APP_GUZZI_COMMAND__NOP) {
        callback(
                app_private,
                app_guzzi_command_dbg_fill(
                        &command
                    )
            );
        /* TODO: non cached? */
        app_guzzi_command_dbg_id = (uint32_t)APP_GUZZI_COMMAND__NOP;
        return 1;
    }

    return 0;
}

void app_guzzi_command_dbg_wait(
        void *app_private,
        app_guzzi_command_callback_t *callback
    )
{
    while (!app_guzzi_command_dbg_peek(app_private, callback)) {
        osal_usleep(APP_GUZZI_COMMAND_DBG_PROBE_INTERVAL_MS*1000);
    }
}

int app_guzzi_command_dbg_wait_timeout(
        void *app_private,
        app_guzzi_command_callback_t *callback,
        uint32_t timeout_ms
    )
{
    uint32_t left, msleep;
    int called;

    left = timeout_ms;

    while (!(called = app_guzzi_command_dbg_peek(app_private, callback)) && left) {
        msleep = APP_GUZZI_COMMAND_DBG_PROBE_INTERVAL_MS < left
            ? APP_GUZZI_COMMAND_DBG_PROBE_INTERVAL_MS
            : left;
        osal_usleep(msleep*1000);
        left -= msleep;
    }

    return called;
}
