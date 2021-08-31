#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

#include <errno.h>

#include "app_guzzi_command.h"
#include "app_guzzi_spi_commands.h"

#define IN_MESSAGE_MAX  64

static uint8_t in_message[IN_MESSAGE_MAX];
static uint8_t in_processing_message[IN_MESSAGE_MAX];
static int in_msg_cnt = 0;
static int in_message_err = 0;
static mqd_t mq_gzz_cmd;

static uint32_t get_dynamic_tuning (app_guzzi_command_t *command, char * data)
{
    char        *next;
    uint32_t    i;
    uint8_t     *dtp_payload = command->live_tuning.data;

    command->id = APP_GUZZI_COMMAND__NOP;
    command->live_tuning.size = strtoul(data, &next, 0);

    if (command->live_tuning.size > sizeof(command->live_tuning.data)) {
        printf("Dynamic dtp size: %d exceeded: %d\n",
                (int)command->live_tuning.size,
                sizeof(command->live_tuning.data)
                );
        goto EXIT_1;
    }

    command->live_tuning.offset = strtoul(next, &next, 0);
    for (i= 0; i < command->live_tuning.size; i ++) {
        *dtp_payload = strtoul(next, &next, 0);
        dtp_payload ++;
    }

    command->id = APP_GUZZI_COMMAND__LIVE_TUNING_APPLY;
    return 0;

EXIT_1:
    return -1;
}

static uint32_t get_tuning_id (app_guzzi_command_t *command, char *data)
{
    // Skip leading white spaces
    while (' ' == *data) {
        data++;
    }

    strncpy((char *)command->live_tuning.data,
            data,
            command->live_tuning.size);

    command->id = APP_GUZZI_COMMAND__LIVE_TUNING_UNLOCK;
    return 0;
}

static app_guzzi_command_t *str_to_app_guzzi_command(
        app_guzzi_command_t *command,
        char *command_srt
    )
{
    uint32_t command_spi_id;
    //uint32_t cammmm_id;
    char *command_spi_params;

    command_spi_id = strtoul(
            command_srt,
            &command_spi_params,
            0
        );

    switch (command_spi_id) {
        case APP_GUZZI_SPI_CMD_START_STREAM:
            command->id = APP_GUZZI_COMMAND__CAM_START;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_STOP_STREAM:
            command->id = APP_GUZZI_COMMAND__CAM_STOP;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_REQ_STILL:
            command->id = APP_GUZZI_COMMAND__CAM_CAPTURE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_MOV_LENS:
            command->id = APP_GUZZI_COMMAND__CAM_LENS_MOVE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.lens_move.pos = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_FOCUS_TRIGGER:
            command->id = APP_GUZZI_COMMAND__CAM_AF_TRIGGER;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_AE_MANUAL:
            command->id = APP_GUZZI_COMMAND__CAM_AE_MANUAL;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_manual.exp_us = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_manual.sensitivity_iso = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_manual.frame_duration_us = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_AE_AUTO:
            command->id = APP_GUZZI_COMMAND__CAM_AE_AUTO;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_SET_AWB_MODE:
            command->id = APP_GUZZI_COMMAND__CAM_AWB_MODE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.awb_mode.mode = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_SCENE_MODES:
            command->id = APP_GUZZI_COMMAND__CAM_SCENE_MODE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.scene_mode.type = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_ANTIBANDING_MODES:
            command->id = APP_GUZZI_COMMAND__CAM_ANTIBANDING_MODE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.antibanding_mode.type = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_AE_LOCK:
            command->id = APP_GUZZI_COMMAND__CAM_AE_LOCK;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_lock_mode.type = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_AE_TARGET_FPS_RANGE:
            command->id = APP_GUZZI_COMMAND__CAM_AE_TARGET_FPS_RANGE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_target_fps_range.min_fps = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_target_fps_range.max_fps = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_AWB_LOCK:
            command->id = APP_GUZZI_COMMAND__CAM_AWB_LOCK;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.awb_lock_control.type = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_CAPTURE_INTENT:
            command->id = APP_GUZZI_COMMAND__CAM_CAPTURE_INTRENT;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.capture_intent.mode = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_CONTROL_MODE:
            command->id = APP_GUZZI_COMMAND__CAM_CONTROL_MODE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.control_mode.type = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_FRAME_DURATION:
            command->id = APP_GUZZI_COMMAND__CAM_FRAME_DURATION;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.frame_duration.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_EXPOSURE_COMPENSATION:
            command->id = APP_GUZZI_COMMAND__CAM_AE_EXPOSURE_COMPENSATION;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.exposure_compensation.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
                    break;
        case APP_GUZZI_SPI_CMD_SENSITIVITY:
            command->id = APP_GUZZI_COMMAND__CAM_SENSITIVITY;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.sensitivity.iso_val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
                    break;
        case APP_GUZZI_SPI_CMD_EFFECT_MODE:
            command->id = APP_GUZZI_COMMAND__CAM_EFFECT_MODE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.effect_mode.type = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_AF_MODE:
            command->id = APP_GUZZI_COMMAND__CAM_AF_MODE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.af_mode.type = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_NOISE_REDUCTION_STRENGTH:
            command->id = APP_GUZZI_COMMAND__CAM_NOISE_REDUCTION_STRENGTH;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.noise_reduction_strength.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_SATURATION:
            command->id = APP_GUZZI_COMMAND__CAM_SATURATION;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.saturation.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_BRIGHTNESS:
            command->id = APP_GUZZI_COMMAND__CAM_BRIGHTNESS;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.brightness.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_STREAM_FORMAT:
            command->id = APP_GUZZI_COMMAND__CAM_FORMAT;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.format.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_CAM_RESOLUTION:
            command->id = APP_GUZZI_COMMAND__CAM_RESOLUTION;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.resolution.width = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.resolution.height = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_SHARPNESS:
            command->id = APP_GUZZI_COMMAND__CAM_SHARPNESS;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.sharpness.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;

        case APP_GUZZI_SPI_LIVE_TUNING_UNLOCK:
            get_tuning_id(command, command_spi_params);
            break;

        case APP_GUZZI_SPI_LIVE_TUNING_APPLY:
            get_dynamic_tuning(command, command_spi_params);
            break;

        case APP_GUZZI_SPI_CMD_CUST_USECASE:
            command->id = APP_GUZZI_COMMAND__CAM_CUST_USECASE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.cust_usecase.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;

        case APP_GUZZI_SPI_CMD_CUST_CAPT_MODE:
            command->id = APP_GUZZI_COMMAND__CAM_CUST_CAPT_MODE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.cust_capt_mode.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;

        case APP_GUZZI_SPI_CMD_CUST_EXP_BRACKETS:
            command->id = APP_GUZZI_COMMAND__CAM_CUST_EXP_BRACKETS;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.cust_exp_brackets.val1 = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.cust_exp_brackets.val2 = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.cust_exp_brackets.val3 = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;

        case APP_GUZZI_SPI_CMD_CUST_CAPTURE:
            command->id = APP_GUZZI_COMMAND__CAM_CUST_CAPTURE;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.cam_cust_capture.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;

        case APP_GUZZI_SPI_CMD_CONTRAST:
            command->id = APP_GUZZI_COMMAND__CAM_CONTRAST;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.contrast.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;

        case APP_GUZZI_SPI_CMD_AE_REGION:
            command->id = APP_GUZZI_COMMAND__CAM_AE_REGION;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_region.x = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_region.y = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_region.w = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_region.h = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_region.priority = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_AF_REGION:
            command->id = APP_GUZZI_COMMAND__CAM_AF_REGION;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_region.x = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_region.y = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_region.w = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_region.h = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.ae_region.priority = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_LUMA_DENOISE:
            command->id = APP_GUZZI_COMMAND__CAM_LUMA_DNS;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.luma_dns.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        case APP_GUZZI_SPI_CMD_CHROMA_DENOISE:
            command->id = APP_GUZZI_COMMAND__CAM_CHROMA_DNS;
            command->cam.id = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            command->cam.chroma_dns.val = strtoul(
                    command_spi_params,
                    &command_spi_params,
                    0
                );
            break;
        default:
            printf(
                    "Unknown APP GUZZI SPI command id: %ld\n",
                    command_spi_id
                );
            command->id = APP_GUZZI_COMMAND__NOP;
    }

    return command;
}

int app_guzzi_command_ext_wait_timeout(
        void *app_private,
        app_guzzi_command_callback_t *callback,
        uint32_t timeout_ms
    )
{
    app_guzzi_command_t cmd_gzz;
    uint32_t nsec, s;
    struct timespec tv;

    clock_gettime(CLOCK_REALTIME, &tv);

    nsec = (tv.tv_nsec + (timeout_ms * 1000)) * 1000;

    if (nsec >= (1000 * 1000 * 1000)) {
        nsec -= (1000 * 1000 * 1000);
        tv.tv_sec += 1;
    }

    s = mq_timedreceive(mq_gzz_cmd, (void *)&cmd_gzz, sizeof(cmd_gzz), 0, &tv);

    if (s == sizeof(cmd_gzz)) {
        callback(app_private, &cmd_gzz);
    }

    return 0;
}

static inline void in_message_init(void)
{
    in_msg_cnt = 0;
    in_message_err = 0;
}

static inline void in_message_process(void)
{
    if (in_msg_cnt < 5)
    {
        in_message_err++;
        printf("Error: incoming message is too short (%d) or protocol error\n", in_msg_cnt);
    }
    if ((in_message[3]+4) != in_msg_cnt)
    {
        in_message_err++;
        printf("Error: protocol error msg_len = %d, but in_msg_cnt = %d\n", (in_message[3]+4), in_msg_cnt);
    }
    if (in_message_err == 0)
    {
        memcpy (in_processing_message, in_message, IN_MESSAGE_MAX);
        app_guzzi_command_t cmd_gzz;
        str_to_app_guzzi_command(&cmd_gzz, (void *)&in_processing_message[4]);
        mq_send (mq_gzz_cmd, (void *)&cmd_gzz, sizeof(cmd_gzz), 0);
    }
    in_message_init();
}

void in_symbol_process(uint16_t in_symbol)
{
    if ((in_symbol&0xFF) != 0xFF)
    {
        if (in_msg_cnt < (IN_MESSAGE_MAX-1))
        {
            in_message[in_msg_cnt] = (uint8_t)in_symbol;
            in_msg_cnt++;
        } else {
            in_message_err++;
            printf("Error: incoming message is too big or protocol error\n");
        }
    }
    if (in_symbol&0xFF00)
    {
        if (in_msg_cnt < IN_MESSAGE_MAX)
        {
            in_message[in_msg_cnt] = 0;
        } else
        {
            in_message[IN_MESSAGE_MAX-1] = 0;
            in_message_err++;
        }
        in_message_process();
    }
}

int32_t ext_cmds_create(void)
{
    struct mq_attr m_attr;
    m_attr.mq_maxmsg  = 8;
    m_attr.mq_msgsize = sizeof(app_guzzi_command_t);

    mq_gzz_cmd = mq_open("uvc_to_guzz" ,O_RDWR | O_CREAT, S_IRWXU | S_IRWXG, &m_attr);
    if (mq_gzz_cmd == (mqd_t)-1) {
        printf("%s:%d Queue creation failure\n", __func__, __LINE__);
        goto exit_1;
    }

exit_1:
    return -1;
}

void ext_cmds_destroy(void)
{
    mq_close(mq_gzz_cmd);
}
