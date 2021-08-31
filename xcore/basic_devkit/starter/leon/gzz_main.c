/*
 * gzz_main.c
 *
 *  Created on: Sep 29, 2017
 */


#include <osal/osal_stdlib.h>
#include <osal/osal_time.h>
#include <osal/osal_string.h>
#include <utils/mms_debug.h>
#include <platform/inc/platform.h>
#include <version_info.h>
#include <guzzi_event/include/guzzi_event.h>
#include <guzzi_event_global/include/guzzi_event_global.h>
#include <dtp/dtp_server_defs.h>
#include <components/camera/vcamera_iface/virt_cm/inc/virt_cm.h>
#include <assert.h>
#include <fcntl.h>
#include <rtems/fb.h>
#include <sys/ioctl.h>
#include <VcsHooksApi.h>
#include <rtems/cpuuse.h>
#include "app_guzzi_command_dbg.h"
#include "camera_control.h"

mmsdbg_define_variable(
        vdl_guzzi_ipipe3_dbg,
        DL_DEFAULT,
        0,
        "vdl_guzzi_ipipe3_dbg",
        "Test ipipe3"
    );
#define MMSDEBUGLEVEL mmsdbg_use_variable(vdl_guzzi_ipipe3_dbg)

dtp_server_hndl_t dtp_srv_hndl;
extern uint8_t ext_dtp_database[];
extern uint8_t ext_dtp_database_end[];

uint32_t __attribute__((section(".cmx_direct.data"))) showCpuReport = 0;

void guzzi_camera3_capture_result__x11_configure_streams(
        int camera_id,
        void *streams
    )
{
    UNUSED(camera_id);
    UNUSED(streams);
}

void guzzi_camera3_capture_result(
        int camera_id,
        unsigned int stream_id,
        unsigned int frame_number,
        void *data,
        unsigned int data_size
    )
{
    UNUSED(camera_id);
    UNUSED(stream_id);
    UNUSED(data);
    UNUSED(frame_number);
    UNUSED(data_size);
}

static void profile_ready_cb(
        profile_t *profile,
        void *prv,
        void *buffer,
        unsigned int buffer_size
    )
{
    UNUSED(prv);
    UNUSED(buffer_size);
    //printf("savefile dump.bin %p %d \n", buffer, buffer_size);
    PROFILE_RELEASE_READY(buffer);
}

#ifndef GZZ_AEWB_MERGER_SWITCH
#error GZZ_AEWB_MERGER_SWITCH is not defined
#endif
uint32 get_cl_aewb_switch (void)
{
    return GZZ_AEWB_MERGER_SWITCH;
}

#ifndef GZZ_MASTER_SLAVE_SELECT
#error GZZ_MASTER_SLAVE_SELECT is not defined
#endif
uint32 get_cl_app_scenario (void)
{
    return GZZ_MASTER_SLAVE_SELECT;
}

void print_ms_cfg(void)
{
    char    *cfg_srt[] =
    {
        "master_all",
        "master_slave_slave",
        "single_master_slave",
        "master_slave_alt",
    };
    uint32  cfg = GZZ_MASTER_SLAVE_SELECT;

    switch (cfg) {
        default:
            cfg = 0;
            /* no break */
        case 0:
        case 1:
        case 2:
        case 3:
            break;
    }
    printf("!!! Master Slave config is: %s !!!\n", cfg_srt[cfg]);
}

char usbIqData[16]= "/dev/fb5IQ";

void send_iq_data (void *data, uint32 size)
{
    int     fd;
    struct fb_var_screeninfo fb_info = {0};

    fb_info.bits_per_pixel  = 8;
    fb_info.xres            = size;
    fb_info.yres            = 1;

    fd = open(usbIqData, O_RDWR);
    if(fd < 0) {
        printf("Error open device: %s\n", usbIqData);
        goto exit1;
    }

    if (0 > ioctl(fd, FBIOPUT_VSCREENINFO, &fb_info)) {
        printf("Error ioctl device: %s\n", usbIqData);
        goto exit2;
    }

    if (0 > write(fd, data, size)) {
        printf("Error write device: %s\n", usbIqData);
    }

exit2:
    if (0 > close(fd)) {
        printf("Error close device: %s\n", usbIqData);
    }

exit1:
    return;
}

/*
 * ****************************************************************************
 * ** App GUZZI Command callback **********************************************
 * ****************************************************************************
 */
static void app_guzzi_command_callback(
        void *app_private,
        app_guzzi_command_t *command
    )
{
    UNUSED(app_private);
    mmsdbg(DL_ERROR, "command->id:%d", command->id);

    if (camera_control_is_active(command->cam.id))
    {
        if (APP_GUZZI_COMMAND__CAM_START == command->id) {
            mmsdbg(DL_ERROR, "Skipping command START for already activated camera %lu", command->cam.id);
            return;
        }
    } else {
        if (APP_GUZZI_COMMAND__CAM_START != command->id) {
            if (APP_GUZZI_COMMAND__CAM_CUST_USECASE == command->id) {
                store_cust_usecase(command->cam.id, command->cam.cust_usecase.val);
            } else {
                mmsdbg(DL_ERROR, "Skipping command %d for Non active camera %d", (int)command->id, (int)command->cam.id);
            }
            return;
        }
    }
    switch (command->id) {
        case APP_GUZZI_COMMAND__NOP:
            //camera_control_start(0);
            break;
        case APP_GUZZI_COMMAND__CAM_START:
            PROFILE_ADD(PROFILE_ID_EXT_START_CMD, 0, 0);
            mmsdbg(DL_ERROR, "command \"%d %lu\" sent\n", command->id, command->cam.id);
            camera_control_start(command->cam.id);
            break;
        case APP_GUZZI_COMMAND__CAM_STOP:
            PROFILE_ADD(PROFILE_ID_EXT_STOP_CMD, 0, 0);
            camera_control_stop(command->cam.id);
            mmsdbg(DL_ERROR, "command \"%d %lu\" sent\n", command->id, command->cam.id);
            break;
        case APP_GUZZI_COMMAND__CAM_CAPTURE:
            PROFILE_ADD(PROFILE_ID_EXT_CAPTURE_CMD, 0, 0);
            camera_control_capture(command->cam.id);
            mmsdbg(DL_ERROR, "command \"%d %lu\" sent\n", command->id, command->cam.id);
            break;
        case APP_GUZZI_COMMAND__CAM_LENS_MOVE:
            PROFILE_ADD(PROFILE_ID_EXT_LENS_MOVE, 0, 0);
            camera_control_lens_move(
                    command->cam.id,
                    command->cam.lens_move.pos
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.lens_move.pos);
            break;
        case APP_GUZZI_COMMAND__CAM_AF_TRIGGER:
            PROFILE_ADD(PROFILE_ID_EXT_LENS_MOVE, 0, 0);
            camera_control_focus_trigger(command->cam.id);
            mmsdbg(DL_ERROR, "command \"%d %lu\" sent\n", command->id, command->cam.id);
            break;
        case APP_GUZZI_COMMAND__CAM_AE_MANUAL:
            camera_control_ae_manual(
                    command->cam.id,
                    command->cam.ae_manual.exp_us,
                    command->cam.ae_manual.sensitivity_iso,
                    command->cam.ae_manual.frame_duration_us
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.ae_manual.exp_us,
                    command->cam.ae_manual.sensitivity_iso, command->cam.ae_manual.frame_duration_us);
            break;
        case APP_GUZZI_COMMAND__CAM_AE_AUTO:
            camera_control_ae_auto(
                    command->cam.id,
                    CAMERA_CONTROL__AE_AUTO__FLASH_MODE__AUTO
                );
            mmsdbg(DL_ERROR, "command \"%d %lu\" sent\n", command->id, command->cam.id);
            break;
        case APP_GUZZI_COMMAND__CAM_AWB_MODE:
            camera_control_awb_mode(
                    command->cam.id,
                    command->cam.awb_mode.mode
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.awb_mode.mode);
            break;
        case APP_GUZZI_COMMAND__CAM_SCENE_MODE:
            camera_control_scene_mode(
                    command->cam.id,
                    command->cam.scene_mode.type
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.scene_mode.type);
            break;
        case APP_GUZZI_COMMAND__CAM_ANTIBANDING_MODE:
            camera_control_antibanding_mode(
                    command->cam.id,
                    command->cam.antibanding_mode.type
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.antibanding_mode.type);
            break;
        case APP_GUZZI_COMMAND__CAM_AE_LOCK:
            camera_control_ae_lock_mode(
                    command->cam.id,
                    command->cam.ae_lock_mode.type
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.ae_lock_mode.type);
            break;
        case APP_GUZZI_COMMAND__CAM_AE_TARGET_FPS_RANGE:
            camera_control_ae_target_fps_range(
                    command->cam.id,
                    command->cam.ae_target_fps_range.min_fps,
                    command->cam.ae_target_fps_range.max_fps
                );
        mmsdbg(DL_ERROR, "command \"%d %lu %lu %lu\" sent\n", command->id,
                command->cam.id, command->cam.ae_target_fps_range.min_fps,
                command->cam.ae_target_fps_range.max_fps);
            break;
        case APP_GUZZI_COMMAND__CAM_AWB_LOCK:
            camera_control_awb_lock_mode(
                    command->cam.id,
                    command->cam.awb_lock_control.type
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.awb_lock_control.type);
            break;
        case APP_GUZZI_COMMAND__CAM_CAPTURE_INTRENT:
            camera_control_capture_intent(
                    command->cam.id,
                    command->cam.capture_intent.mode
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.capture_intent.mode);
            break;
        case APP_GUZZI_COMMAND__CAM_CONTROL_MODE:
            camera_control_mode(
                    command->cam.id,
                    command->cam.control_mode.type
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.control_mode.type);
            break;
        case APP_GUZZI_COMMAND__CAM_FRAME_DURATION:
            camera_control_frame_duration(
                    command->cam.id,
                    command->cam.frame_duration.val
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %llu\" sent\n",
                    command->id, command->cam.id, command->cam.frame_duration.val);
            break;
        case APP_GUZZI_COMMAND__CAM_AE_EXPOSURE_COMPENSATION:
            camera_control_exp_compensation(
                    command->cam.id,
                    command->cam.exposure_compensation.val
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.exposure_compensation.val);
            break;
        case APP_GUZZI_COMMAND__CAM_SENSITIVITY:
            camera_control_sensitivity(
                    command->cam.id,
                    command->cam.sensitivity.iso_val
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.sensitivity.iso_val);
            break;
        case APP_GUZZI_COMMAND__CAM_EFFECT_MODE:
            camera_control_effect_mode(
                    command->cam.id,
                    command->cam.effect_mode.type
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.effect_mode.type);
            break;
        case APP_GUZZI_COMMAND__CAM_AF_MODE:
            camera_control_af_mode(
                    command->cam.id,
                    command->cam.af_mode.type
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.af_mode.type);
            break;
        case APP_GUZZI_COMMAND__CAM_NOISE_REDUCTION_STRENGTH:
            camera_control_noise_reduction_strength(
                    command->cam.id,
                    command->cam.noise_reduction_strength.val
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.noise_reduction_strength.val);
            break;
        case APP_GUZZI_COMMAND__CAM_SATURATION:
            camera_control_saturation(
                    command->cam.id,
                    command->cam.saturation.val
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.saturation.val);
            break;
        case APP_GUZZI_COMMAND__CAM_BRIGHTNESS:
            camera_control_brightness(
                    command->cam.id,
                    command->cam.brightness.val
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.brightness.val);
            break;
        case APP_GUZZI_COMMAND__CAM_FORMAT:
            camera_control_format(
                    command->cam.id,
                    command->cam.format.val
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.format.val);
            break;
        case APP_GUZZI_COMMAND__CAM_RESOLUTION:
            camera_control_resolution(
                    command->cam.id,
                    command->cam.resolution.width,
                    command->cam.resolution.height
                );
            mmsdbg(DL_ERROR, "command \"%d %lu %lu %lu\" sent\n", command->id,
                    command->cam.id, command->cam.resolution.width,
                    command->cam.resolution.height);
            break;
        case APP_GUZZI_COMMAND__CAM_SHARPNESS:
            camera_control_sharpness(
                    command->cam.id,
                    command->cam.sharpness.val
                );
            mmsdbg(DL_ERROR, "command \"%d  %lu %lu\" sent\n",
                    command->id, command->cam.id, command->cam.sharpness.val);
            break;
        case APP_GUZZI_COMMAND__CAM_CUST_USECASE:
            camera_control_cust_usecase(command->cam.id, command->cam.cust_usecase.val);
            break;
        case APP_GUZZI_COMMAND__CAM_CUST_CAPT_MODE:
            camera_control_cust_capt_mode(command->cam.id, command->cam.cust_capt_mode.val);
            break;
        case APP_GUZZI_COMMAND__CAM_CUST_EXP_BRACKETS:
            camera_control_cust_exp_brackets(command->cam.id,
                                             command->cam.cust_exp_brackets.val1,
                                             command->cam.cust_exp_brackets.val2,
                                             command->cam.cust_exp_brackets.val3);
            break;
        case APP_GUZZI_COMMAND__CAM_CUST_CAPTURE:
            camera_control_cust_capture(command->cam.id, command->cam.cam_cust_capture.val);
            break;
        case APP_GUZZI_COMMAND__CAM_CONTRAST:
            camera_control_contrast(command->cam.id,
                                            command->cam.contrast.val
                                            );
            break;

        case APP_GUZZI_COMMAND__CAM_AE_REGION:
            camera_control_ae_set_priority(command->cam.id,
                                            command->cam.ae_region.x,
                                            command->cam.ae_region.y,
                                            command->cam.ae_region.w,
                                            command->cam.ae_region.h,
                                            command->cam.ae_region.priority
                                            );
            break;
        case  APP_GUZZI_COMMAND__CAM_AF_REGION:
            camera_control_af_set_region(command->cam.id,
                                            command->cam.af_region.x,
                                            command->cam.af_region.y,
                                            command->cam.af_region.w,
                                            command->cam.af_region.h,
                                            command->cam.af_region.priority
                                            );
            break;
        case APP_GUZZI_COMMAND__CAM_LUMA_DNS:
            camera_control_luma_dns(
                    command->cam.id,
                    command->cam.luma_dns.val
                );
            break;
        case APP_GUZZI_COMMAND__CAM_CHROMA_DNS:
            camera_control_chroma_dns(
                    command->cam.id,
                    command->cam.chroma_dns.val
                );
            break;
        default:
            mmsdbg(DL_ERROR, "Unknown App GUZZI Command: %d", command->id);
    }
}

/*
 * ****************************************************************************
 * ** Temp observe function ***************************************************
 * ****************************************************************************
 */
/* TODO: Implement this in board/platform dependent part  */
int app_guzzi_command_wait_timeout(
        void *app_private,
        app_guzzi_command_callback_t *callback,
        uint32_t timeout_ms
    )
{
//    return app_guzzi_command_spi_wait_timeout(app_private, callback, timeout_ms)
//         + app_guzzi_command_dbg_peek(app_private, callback);

    int app_guzzi_command_ext_wait_timeout(
            void *app_private,
            app_guzzi_command_callback_t *callback,
            uint32_t timeout_ms
        );

    return app_guzzi_command_dbg_wait_timeout(app_private, callback, timeout_ms) +
            app_guzzi_command_ext_wait_timeout(app_private, callback, timeout_ms);
}

extern uint32_t app_guzzi_command_dbg_id;
extern uint32_t app_guzzi_command_dbg_cam_id;
void gzz_init (void)
{
     version_info_init();

     osal_init();
     print_ms_cfg();
     dtpsrv_create(&dtp_srv_hndl);
     dtpsrv_import_db(
             dtp_srv_hndl,
             ext_dtp_database,
             ext_dtp_database_end - ext_dtp_database
         );
     PROFILE_INIT(4096, 2, profile_ready_cb, NULL);

     guzzi_platform_init();
     guzzi_event_global_ctreate();

     int32_t ext_cmds_create(void);
     ext_cmds_create();

//     assert(0);
     /// trigger AF
    app_guzzi_command_dbg_cam_id = 0;
    app_guzzi_command_dbg_id = APP_GUZZI_COMMAND__CAM_START;
    app_guzzi_command_wait_timeout( NULL, app_guzzi_command_callback, 10);

    app_guzzi_command_dbg_id = APP_GUZZI_COMMAND__CAM_AF_TRIGGER;
    app_guzzi_command_wait_timeout( NULL, app_guzzi_command_callback, 10);

     for (;;) {
         app_guzzi_command_wait_timeout(
                 NULL,
                 app_guzzi_command_callback,
                 10);
         if(showCpuReport)
         {
             rtems_cpu_usage_report();
             rtems_cpu_usage_reset();
             showCpuReport = 0;
             sleep(5);
         }
     }

     void ext_cmds_destroy(void);
     ext_cmds_destroy();
}
