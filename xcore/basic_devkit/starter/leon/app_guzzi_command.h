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
* @file app_guzzi_command.h
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

#ifndef _APP_GUZZI_COMMAND_H
#define _APP_GUZZI_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    APP_GUZZI_COMMAND__NOP, //0
    APP_GUZZI_COMMAND__CAM_START, //1
    APP_GUZZI_COMMAND__CAM_STOP, //2
    APP_GUZZI_COMMAND__CAM_CAPTURE, //3
    APP_GUZZI_COMMAND__CAM_LENS_MOVE, //4
    APP_GUZZI_COMMAND__CAM_AF_TRIGGER, //5
    APP_GUZZI_COMMAND__CAM_AE_MANUAL, //6
    APP_GUZZI_COMMAND__CAM_AE_AUTO, //7
    APP_GUZZI_COMMAND__CAM_AWB_MODE, //8
    APP_GUZZI_COMMAND__CAM_SCENE_MODE, //9
    APP_GUZZI_COMMAND__CAM_ANTIBANDING_MODE, //10
    APP_GUZZI_COMMAND__CAM_AE_EXPOSURE_COMPENSATION, //11
    sth_else2, //12
    APP_GUZZI_COMMAND__CAM_AE_LOCK, //13
    APP_GUZZI_COMMAND__CAM_AE_TARGET_FPS_RANGE, //14
    sth_else3, //15
    APP_GUZZI_COMMAND__CAM_AWB_LOCK, //16
    APP_GUZZI_COMMAND__CAM_CAPTURE_INTRENT, //17
    APP_GUZZI_COMMAND__CAM_CONTROL_MODE, //18
    sth_else4, //19
    sth_else5, //20
    APP_GUZZI_COMMAND__CAM_FRAME_DURATION, //21
    sth_else7, //22
    APP_GUZZI_COMMAND__CAM_SENSITIVITY, //23
    APP_GUZZI_COMMAND__CAM_EFFECT_MODE, //24
    sth_else8, //25
    APP_GUZZI_COMMAND__CAM_AF_MODE, //26
    APP_GUZZI_COMMAND__CAM_NOISE_REDUCTION_STRENGTH, //27
    APP_GUZZI_COMMAND__CAM_SATURATION, //28
    sth_else9, //29
    sth_else10, //30
    APP_GUZZI_COMMAND__CAM_BRIGHTNESS, //31
    sth_else11, //32
    APP_GUZZI_COMMAND__CAM_FORMAT, //33
    APP_GUZZI_COMMAND__CAM_RESOLUTION, //34
    APP_GUZZI_COMMAND__CAM_SHARPNESS, //35
    APP_GUZZI_COMMAND__LIVE_TUNING_UNLOCK, //36
    APP_GUZZI_COMMAND__LIVE_TUNING_APPLY, //37

    APP_GUZZI_COMMAND__CAM_AE_MERGER,   //38
    APP_GUZZI_COMMAND__CAM_AWB_MERGER,  //39

    APP_GUZZI_COMMAND__CAM_CUST_USECASE,        //40
    APP_GUZZI_COMMAND__CAM_CUST_CAPT_MODE,     //41
    APP_GUZZI_COMMAND__CAM_CUST_EXP_BRACKETS,  //42
    APP_GUZZI_COMMAND__CAM_CUST_CAPTURE,       //43

    APP_GUZZI_COMMAND__CAM_CONTRAST,           //44
    APP_GUZZI_COMMAND__CAM_AE_REGION,          //45
    APP_GUZZI_COMMAND__CAM_AF_REGION,          //46
    APP_GUZZI_COMMAND__CAM_LUMA_DNS,           //47
    APP_GUZZI_COMMAND__CAM_CHROMA_DNS,         //48
    APP_GUZZI_COMMAND__MAX
} app_guzzi_command_id_t;

typedef enum {
    APP_GUZZI_RESPONSE_NOP,
    APP_GUZZI_RESPONSE_SUCCESS,
    APP_GUZZI_RESPONSE_WRONG_COMMAND,
    APP_GUZZI_RESPONSE_CRC_CHECK_FAIL,
    APP_GUZZI_RESPONSE_WRONG_DTP_CHECKSUM,
    APP_GUZZI_RESPONSE_WRONG_DTP_SIZE,
    APP_GUZZI_RESPONSE_MAX
} app_guzzi_response_id_t;

typedef struct __attribute__((packed)){
    app_guzzi_command_id_t id;
    union {
        struct  __attribute__((packed)){
            uint32_t id;
            union {
                struct {
                    uint32_t pos;
                } lens_move;
                struct {
                    uint32_t exp_us;
                    uint32_t sensitivity_iso;
                    uint32_t frame_duration_us;
                } ae_manual;
                struct {
                    uint32_t mode;
                } awb_mode;
                struct {
                    uint32_t type;
                } scene_mode;
                struct {
                    uint32_t type;
                } antibanding_mode;
                struct {
                    uint32_t type;
                } ae_lock_mode;
                struct {
                    uint32_t min_fps;
                    uint32_t max_fps;
                } ae_target_fps_range;
                struct {
                    uint32_t type;
                } awb_lock_control;
                struct {
                    uint32_t mode;
                } capture_intent;
                struct {
                    uint32_t type;
                } control_mode;
                struct {
                    uint64_t val;
                } frame_duration;
                struct {
                    uint32_t val;
                } exposure_compensation;
                struct {
                    uint32_t iso_val;
                } sensitivity;
                struct {
                    uint32_t type;
                } effect_mode;
                struct {
                    uint32_t type;
                } af_mode;
                struct {
                    uint32_t val;
                } noise_reduction_strength;
                struct {
                    uint32_t val;
                } saturation;
                struct {
                    uint32_t val;
                } brightness;
                struct {
                    uint32_t val;
                } format;
                struct {
                    uint32_t width;
                    uint32_t height;
                } resolution;
                struct {
                    uint32_t val;
                } sharpness;
                struct {
                    uint32_t val;
                } cust_usecase;

                struct {
                    uint32_t val;
                } cust_capt_mode;

                struct {
                    uint32_t val1;
                    uint32_t val2;
                    uint32_t val3;
                } cust_exp_brackets;

                struct {
                    uint32_t val;
                } cam_cust_capture;

                struct {
                    int32_t val;
                } contrast;

                struct {
                    uint32_t x;
                    uint32_t y;
                    uint32_t w;
                    uint32_t h;
                    uint32_t priority;
                } ae_region;
                struct {
                    uint32_t x;
                    uint32_t y;
                    uint32_t w;
                    uint32_t h;
                    uint32_t priority;
                } af_region;
                struct {
                    uint32_t val;
                } luma_dns;
                struct {
                    uint32_t val;
                } chroma_dns;
            };
        } cam;
        struct  __attribute__((packed)){
            uint32_t size;
            uint32_t offset;
            uint8_t *data;
            uint32_t crc;
        } live_tuning;
    };
} app_guzzi_command_t;

typedef struct {
    app_guzzi_command_id_t id;
    uint32_t payload_size;
    app_guzzi_response_id_t response;
    int32_t crc;
} app_guzzi_response_t;

typedef void app_guzzi_command_callback_t(
        void *app_private,
        app_guzzi_command_t *command
    );

int app_guzzi_command_peek(
        void *app_private,
        app_guzzi_command_callback_t *callback
    );
void app_guzzi_command_wait(
        void *app_private,
        app_guzzi_command_callback_t *callback
    );
int app_guzzi_command_wait_timeout(
        void *app_private,
        app_guzzi_command_callback_t *callback,
        uint32_t timeout_ms
    );

#ifdef __cplusplus
}
#endif

#endif /* _APP_GUZZI_COMMAND_H */
