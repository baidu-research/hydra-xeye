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
* @file app_guzzi_spi_commands.h
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

#ifndef _APP_GUZZI_SPI_COMMANDS_H
#define _APP_GUZZI_SPI_COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The SPI command is a string:
 *   cmd arg1 arg2...argN
 * All camera commnds begin with camera_id as arg1
 */

#define APP_GUZZI_SPI_CMD_START_STREAM  0 /* arg1 - camera_id */
#define APP_GUZZI_SPI_CMD_STOP_STREAM   1 /* arg1 - camera_id */
#define APP_GUZZI_SPI_CMD_REQ_STILL     2 /* arg1 - camera_id */
#define APP_GUZZI_SPI_CMD_MOV_LENS      3 /* arg1 - camera_id
                                           * arg2 - lens pos 0-255
                                           */
#define APP_GUZZI_SPI_CMD_FOCUS_TRIGGER 4 /* arg1 - camera_id */
#define APP_GUZZI_SPI_CMD_AE_MANUAL     5 /* arg1 - camera_id
                                           * arg2 - exp. time [us]
                                           * arg3 - sensitivity [iso]
                                           * arg4 - frame duration [us]
                                           */
#define APP_GUZZI_SPI_CMD_AE_AUTO       6 /* arg1 - camera_id */
#define APP_GUZZI_SPI_CMD_SET_AWB_MODE  7 /* arg1 - camera_id */
                                          /* arg2 - awb_mode */
#define APP_GUZZI_SPI_CMD_SCENE_MODES   8 /* arg1 - camera_id
                                           * arg2 - scene_mode
                                           */
#define APP_GUZZI_SPI_CMD_ANTIBANDING_MODES      9 /* arg1 - camera_id
                                                    * arg2 - antibanding_mode
                                                    */
#define APP_GUZZI_SPI_CMD_EXPOSURE_COMPENSATION 10 /* arg1 - camera_id
                                                    * arg2 - value
                                                    */
#define APP_GUZZI_SPI_CMD_AE_LOCK       12 /* arg1 - camera_id
                                            * arg2 - ae_lock_mode
                                            */
#define APP_GUZZI_SPI_CMD_AE_TARGET_FPS_RANGE    13 /* arg1 - camera_id
                                                     * arg2 - min_fps
                                                     * arg3 - max_fps
                                                     */
#define APP_GUZZI_SPI_CMD_AWB_LOCK      15 /* arg1 - camera_id
                                            * arg2 - awb_lock_control
                                            */
#define APP_GUZZI_SPI_CMD_CAPTURE_INTENT 16 /* arg1 - camera_id
                                             * arg2 - capture_intent_mode
                                             */
#define APP_GUZZI_SPI_CMD_CONTROL_MODE  17 /* arg1 - camera_id
                                            * arg2 - control_mode
                                            */
#define APP_GUZZI_SPI_CMD_FRAME_DURATION 20 /* arg1 - camera_id
                                             * arg2 - frame_duration
                                             */
#define APP_GUZZI_SPI_CMD_SENSITIVITY   22 /* arg1 - camera_id
                                            * arg2 - control_mode
                                            */
#define APP_GUZZI_SPI_CMD_EFFECT_MODE   23 /* arg1 - camera_id
                                            * arg2 - effect_mode
                                            */
#define APP_GUZZI_SPI_CMD_AF_MODE       25 /* arg1 - camera_id
                                            * arg2 - af_mode
                                            */
#define APP_GUZZI_SPI_CMD_NOISE_REDUCTION_STRENGTH 26 /* arg1 - camera_id
                                                       * arg2 - value
                                                       */
#define APP_GUZZI_SPI_CMD_SATURATION    27 /* arg1 - camera_id
                                            * arg2 - value
                                            */
#define APP_GUZZI_SPI_CMD_BRIGHTNESS    30 /* arg1 - camera_id
                                            * arg2 - value
                                            */
#define APP_GUZZI_SPI_CMD_STREAM_FORMAT 32 /* arg1 - camera_id
                                            * arg2 - value
                                            */
#define APP_GUZZI_SPI_CMD_CAM_RESOLUTION 33 /* arg1 - camera_id
                                             * arg2 - width
                                             * arg3 - height
                                             */
#define APP_GUZZI_SPI_CMD_SHARPNESS     34 /* arg1 - camera_id
                                            * arg2 - value
                                            */
#define APP_GUZZI_SPI_LIVE_TUNING_UNLOCK    35 /* arg1 - id
                                             */
#define APP_GUZZI_SPI_LIVE_TUNING_APPLY     36 /* arg1 - size
                                             * arg2 - offset
                                             * arg3 - arg size-1 - data
                                             */

#define APP_GUZZI_SPI_CMD_CUST_USECASE      39
#define APP_GUZZI_SPI_CMD_CUST_CAPT_MODE    40
#define APP_GUZZI_SPI_CMD_CUST_EXP_BRACKETS 41
#define APP_GUZZI_SPI_CMD_CUST_CAPTURE      42

#define APP_GUZZI_SPI_CMD_CONTRAST          43
#define APP_GUZZI_SPI_CMD_AE_REGION         44
#define APP_GUZZI_SPI_CMD_AF_REGION         46
#define APP_GUZZI_SPI_CMD_LUMA_DENOISE      47 /* arg1 - camera_id
                                             * arg2 - value
                                             */
#define APP_GUZZI_SPI_CMD_CHROMA_DENOISE    48 /* arg1 - camera_id
                                             * arg2 - value
                                             */

#ifdef __cplusplus
}
#endif

#endif /* _APP_GUZZI_SPI_COMMANDS_H */
