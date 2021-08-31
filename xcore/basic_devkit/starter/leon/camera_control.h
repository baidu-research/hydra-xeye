/* =============================================================================
* Copyright (c) 2013-2014 MM Solutions AD
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
* @file camera_control.h
*
* @author ( MM Solutions AD )
*
*/
/* -----------------------------------------------------------------------------
*!
*! Revision History
*! ===================================
*! 05-Nov-2013 : Author ( MM Solutions AD )
*! Created
* =========================================================================== */

#ifndef _CAMERA_CONTROL_H
#define _CAMERA_CONTROL_H

#include <stdint.h>

enum {
    CAMERA_CONTROL__AE_AUTO__FLASH_MODE__OFF,
    CAMERA_CONTROL__AE_AUTO__FLASH_MODE__AUTO,
    CAMERA_CONTROL__AE_AUTO__FLASH_MODE__ALWAYS,
    CAMERA_CONTROL__AE_AUTO__FLASH_MODE__AUTO_REDEYE,
};

enum {
    CAMERA_CONTROL__AWB_MODE__OFF, //0
    CAMERA_CONTROL__AWB_MODE__AUTO, //1
    CAMERA_CONTROL__AWB_MODE__INCANDESCENT, //2
    CAMERA_CONTROL__AWB_MODE__FLUORESCENT, //3
    CAMERA_CONTROL__AWB_MODE__WARM_FLUORESCENT, //4
    CAMERA_CONTROL__AWB_MODE__DAYLIGHT, //5
    CAMERA_CONTROL__AWB_MODE__CLOUDY_DAYLIGHT, //6
    CAMERA_CONTROL__AWB_MODE__TWILIGHT, //7
    CAMERA_CONTROL__AWB_MODE__SHADE, //8
};

enum {
    CAMERA_CONTROL__SCENE_MODE__UNSUPPORTED, //0
    CAMERA_CONTROL__SCENE_MODE__FACE_PRIORITY, //1
    CAMERA_CONTROL__SCENE_MODE__ACTION, //2
    CAMERA_CONTROL__SCENE_MODE__PORTRAIT, //3
    CAMERA_CONTROL__SCENE_MODE__LANDSCAPE, //4
    CAMERA_CONTROL__SCENE_MODE__NIGHT, //5
    CAMERA_CONTROL__SCENE_MODE__NIGHT_PORTRAIT, //6
    CAMERA_CONTROL__SCENE_MODE__THEATRE, //7
    CAMERA_CONTROL__SCENE_MODE__BEACH, //8
    CAMERA_CONTROL__SCENE_MODE__SNOW, //9
    CAMERA_CONTROL__SCENE_MODE__SUNSET, //10
    CAMERA_CONTROL__SCENE_MODE__STEADYPHOTO, //11
    CAMERA_CONTROL__SCENE_MODE__FIREWORKS, //12
    CAMERA_CONTROL__SCENE_MODE__SPORTS, //13
    CAMERA_CONTROL__SCENE_MODE__PARTY, //14
    CAMERA_CONTROL__SCENE_MODE__CANDLELIGHT, //15
    CAMERA_CONTROL__SCENE_MODE__BARCODE, //16
};

enum {
    CAMERA_CONTROL__AE_ANTIBANDING_MODE__OFF, //0
    CAMERA_CONTROL__AE_ANTIBANDING_MODE__50HZ, //1
    CAMERA_CONTROL__AE_ANTIBANDING_MODE__60HZ, //2
    CAMERA_CONTROL__AE_ANTIBANDING_MODE__AUTO, //3
};

enum {
    CAMERA_CONTROL__AE_LOCK__OFF, //0
    CAMERA_CONTROL__AE_LOCK__ON, //1
};

enum {
    CAMERA_CONTROL__AWB_LOCK__OFF, //0
    CAMERA_CONTROL__AWB_LOCK__ON, //1
};

enum {
    CAMERA_CONTROL__CAPTURE_INTENT__CUSTOM, //0
    CAMERA_CONTROL__CAPTURE_INTENT__PREVIEW, //1
    CAMERA_CONTROL__CAPTURE_INTENT__STILL_CAPTURE, //2
    CAMERA_CONTROL__CAPTURE_INTENT__VIDEO_RECORD, //3
    CAMERA_CONTROL__CAPTURE_INTENT__VIDEO_SNAPSHOT, //4
    CAMERA_CONTROL__CAPTURE_INTENT__ZERO_SHUTTER_LAG, //5
};

enum {
    CAMERA_CONTROL__MODE__OFF, //0
    CAMERA_CONTROL__MODE__AUTO, //1
    CAMERA_CONTROL__MODE__USE_SCENE_MODE, //2
};

enum {
    CAMERA_CONTROL__EFFECT_MODE__OFF, //0
    CAMERA_CONTROL__EFFECT_MODE__MONO, //1
    CAMERA_CONTROL__EFFECT_MODE__NEGATIVE, //2
    CAMERA_CONTROL__EFFECT_MODE__SOLARIZE, //3
    CAMERA_CONTROL__EFFECT_MODE__SEPIA, //4
    CAMERA_CONTROL__EFFECT_MODE__POSTERIZE, //5
    CAMERA_CONTROL__EFFECT_MODE__WHITEBOARD, //6
    CAMERA_CONTROL__EFFECT_MODE__BLACKBOARD, //7
    CAMERA_CONTROL__EFFECT_MODE__AQUA, //8
};

enum {
    CAMERA_CONTROL__AF_MODE_OFF, //0
    CAMERA_CONTROL__AF_MODE_AUTO, //1
    CAMERA_CONTROL__AF_MODE_MACRO, //2
    CAMERA_CONTROL__AF_MODE_CONTINUOUS_VIDEO, //3
    CAMERA_CONTROL__AF_MODE_CONTINUOUS_PICTURE, //4
    CAMERA_CONTROL__AF_MODE_EDOF, //5
};

/* return 0 if camera is NOT created */
uint32_t camera_control_is_active(int cam_id);
void store_cust_usecase(int cam_id, int usecase);
void camera_control_start(int cam_id);
void camera_control_stop(int cam_id);
void camera_control_capture(int cam_id);
void camera_control_lens_move(int cam_id, int lens_position);
void camera_control_focus_trigger(int cam_id);
void camera_control_ae_manual(
        int cam_id, 
        uint32_t time_us,
        uint32_t sensitivity_iso,
        uint32_t frame_duration_us
    );
void camera_control_ae_auto(int cam_id, uint32_t flash_mode);
void camera_control_awb_mode(int cam_id, uint32_t awb_mode);
void camera_control_scene_mode(int cam_id, uint32_t scene_mode);
void camera_control_antibanding_mode(int cam_id, uint32_t antibanding_mode);
void camera_control_ae_lock_mode(int cam_id, uint32_t ae_lock_mode);
void camera_control_ae_target_fps_range(int cam_id, uint32_t min_fps, uint32_t max_fps);
void camera_control_awb_lock_mode(int cam_id, uint32_t awb_lock_control);
void camera_control_capture_intent(int cam_id, uint32_t capture_intent_mode);
void camera_control_mode(int cam_id, uint32_t control_mode);
void camera_control_frame_duration(int cam_id, uint64_t frame_duration);
void camera_control_exp_compensation(int cam_id, uint32_t exp_compensation);
void camera_control_sensitivity(int cam_id, uint32_t iso_val);
void camera_control_effect_mode(int cam_id, uint32_t effect_mode);
void camera_control_af_mode(int cam_id, uint32_t af_mode);
void camera_control_noise_reduction_strength(int cam_id, uint32_t strength_val);
void camera_control_saturation(int cam_id, uint32_t saturation_val);
void camera_control_brightness(int cam_id, uint32_t brightness_val);
void camera_control_format(int cam_id, uint32_t format_type);
void camera_control_resolution(int cam_id, uint32_t width, uint32_t height);
void camera_control_sharpness(int cam_id, uint32_t sharpness_val);
void camera_control_luma_dns(int cam_id, uint32_t luma_dns_val);
void camera_control_chroma_dns(int cam_id, uint32_t chroma_dns_val);
void camera_control_cust_usecase(int cam_id, uint32_t usecase);
void camera_control_cust_capt_mode(int cam_id, uint32_t capt_mode);
void camera_control_cust_exp_brackets(int cam_id, uint32_t v1, uint32_t v2, uint32_t v3);
void camera_control_cust_capture(int cam_id, uint32_t num_captures);
void camera_control_contrast(int cam_id, int32_t contrast);
void camera_control_ae_set_priority(int cam_id,
                                        uint32_t x,
                                        uint32_t y,
                                        uint32_t w,
                                        uint32_t h,
                                        uint32_t p);
void camera_control_af_set_region(int cam_id,
                                        uint32_t x,
                                        uint32_t y,
                                        uint32_t w,
                                        uint32_t h,
                                        uint32_t p);

#endif /* _CAMERA_CONTROL_H */

