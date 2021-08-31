/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "dl_input_thread.h"
#include "dl_process.h"
#include "xeye_util.h"
#include "debug_def.h"

#include <unistd.h>
#include <assert.h>
#include <string.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

bool DlInputThread::_s_running = false;

DlInputThread::DlInputThread() {
    _xeye_config = NULL;
    _input_queue = NULL;
    _connector = NULL;
    _work_thread = NULL;
    DlInputThread::_s_running = false;
}

DlInputThread::~DlInputThread() {
    uninit();
}

int DlInputThread::init(UsbConnector *connector, DlInputQueue *input_queue,
        XeyeConfig *config) {
    if (NULL == connector || NULL == input_queue || NULL == config) {
        printf("DlInputThread::init invalid parameters.\n");
        return -1;
    }

    _xeye_config = config;
    _input_queue = input_queue;
    _connector = connector;
    return 0;
}

void DlInputThread::uninit() {
    stop();
    _xeye_config = NULL;
    _input_queue = NULL;
    _connector = NULL;
}

int DlInputThread::start() {
    if (DlInputThread::_s_running) {
        printf("dl input thread is already running.\n");
        return -1;
    }

    DlInputThread::_s_running = true;
    _work_thread = new std::thread(DlInputThread::s_thread_func, _connector, \
                                   _input_queue, _xeye_config);
    return 0;
}

int DlInputThread::stop() {
    if (!DlInputThread::_s_running) {
        printf("dl input thread is not running.\n");
        return -1;
    }

    if (NULL != _work_thread) {
        DlInputThread::_s_running = false;
        _work_thread->join();
        delete _work_thread;
        _work_thread = NULL;
    }
    return 0;
}

void DlInputThread::s_thread_func(UsbConnector *connector, \
                                  DlInputQueue *input_queue, \
                                  XeyeConfig *config) {
    assert(NULL != config);
    assert(NULL != input_queue);
    assert(NULL != connector);

    while (DlInputThread::_s_running) {
        if (config->xeye_mode != XEYEMODE_COMPUTATION) {
            printf("DlInputThread xeye_mode:%d\n", config->xeye_mode);
            sleep(5);
            continue;
        }

        XeyeDataItem data_item;
        int ret = input_queue->pop(data_item);
        if (0 != ret) {
            //printf("input_queue is empty.\n");
            usleep(10000); // 10ms
            continue;
        }

        uint8_t *message_buffer = NULL;
        do {
            // int ret = DlProcess::preprocess(*config, data_item);
            // if (0 != ret) {
            //     printf("preprocess failed. ret:%d\n", ret);
            //     ret = -1;
            //     break;
            // }

            message_buffer = (uint8_t *)malloc(XEYE_MESSAGE_SIZE);
            if (NULL == message_buffer) {
                printf("failed to malloc message_buffer.\n");
                ret = -2;
                break;
            }
            memset(message_buffer, 0, XEYE_MESSAGE_SIZE);

            data_item.xeye_message = (XeyeMessage *)message_buffer;
            XeyeMessage &xeye_message = *data_item.xeye_message;
            xeye_message.magic = XEYE_MAGIC;
            xeye_message.sender = CONNECTTARGET_HOST;
            xeye_message.receiver = CONNECTTARGET_XEYE;
            xeye_message.message_type = MESSAGETYPE_SET_INPUT;
            xeye_message.with_config = 0;
            xeye_message.model_count = 0;
            xeye_message.image_count = 1;
            xeye_message.with_result = 0;

            ImageInfo &image_info = xeye_message.image_info[0];
            XeyeImage &xeye_image = data_item.xeye_image;
            image_info.timestamp = xeye_image.timestamp;
            image_info.image_type = xeye_image.image_type;
            assert(IMAGETYPE_RGBRGB == image_info.image_type);
            image_info.image_data_type = xeye_image.image_data_type;
            assert(IMAGEDATATYPE_FP16 == image_info.image_data_type);
            image_info.size = 720 * 1280 * 3 / 2;
            image_info.height = 720;
            image_info.width = 1280;
            memcpy(image_info.extra, xeye_image.extra, EXTRA_IMAGE_INFO_SIZE);

            //send MESSAGETYPE_SET_INPUT to xeye
            //printf("######## DlInputThread before send message.\n");
            ret = connector->send(message_buffer, XEYE_MESSAGE_SIZE, \
                                  XEYE_VSC_TIMEOUT);
            //printf("######## DlInputThread after send message.\n");
            if (0 == ret) {
                DEBUG_PRINT("send MESSAGETYPE_SET_INPUT successfully!\n");
            } else if (1 == ret) {
                DEBUG_PRINT("send MESSAGETYPE_SET_INPUT timeout.\n");
                ret = -3;
                break;
            } else {
                printf("warning: failed to send MESSAGETYPE_SET_INPUT. "\
                       "ret:%d\n", ret);
                usleep(10000);
                ret = -4;
                break;
            }

            //send image to xeye
            //printf("######## DlInputThread before send image.\n");
            ret = connector->send((uint8_t *)xeye_image.org_image, \
                                  image_info.size, XEYE_VSC_TIMEOUT);
            //printf("######## DlInputThread after send image.\n");
            if (0 == ret) {
                DEBUG_PRINT("send image successfully!\n");
            } else if (1 == ret) {
                DEBUG_PRINT("send image timeout.\n");
                ret = -5;
                break;
            } else {
                printf("warning: failed to send image. ret:%d\n", \
                       ret);
                usleep(10000);
                ret = -6;
                break;
            }
        } while (false);

        free(message_buffer);
        message_buffer = NULL;
        data_item.xeye_message = NULL;

        XeyeImage &xeye_image = data_item.xeye_image;
        if (NULL != xeye_image.org_mat.data) {
            xeye_image.org_mat.release();
            xeye_image.org_image = NULL;
        } else if (NULL != xeye_image.org_image) {
            free(xeye_image.org_image);
            xeye_image.org_image = NULL;
        } else if (NULL != xeye_image.float_image) {
            //printf("free float_image.\n");
            free(xeye_image.float_image);
            xeye_image.float_image = NULL;
        } else {
            printf("input thread should not execute here.\n");
        }
        //free image that sent to xeye
        free(xeye_image.image);
        xeye_image.image = NULL;
        assert(NULL == data_item.xeye_result);
    }
}
