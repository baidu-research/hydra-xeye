/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "connector_message_thread.h"
#include "xeye_util.h"
#include "debug_def.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

bool ConnectorMessageThread::_s_running = false;

ConnectorMessageThread::ConnectorMessageThread() {
    _input_queue = NULL;
    _output_queue = NULL;
    _connector = NULL;
    _xeye_config = NULL;
    _work_thread = NULL;
    ConnectorMessageThread::_s_running = false;
}

ConnectorMessageThread::~ConnectorMessageThread() {
    uninit();
}

int ConnectorMessageThread::init(UsbConnector *connector, \
                                 DlInputQueue *input_queue, \
                                 DlOutputQueue *output_queue, \
                                 XeyeConfig *xeye_config) {
    if (NULL == connector || NULL == input_queue || NULL == output_queue || \
            NULL == xeye_config) {
        printf("ConnectorMessageThread::init invalid parameters.\n");
        return -1;
    }

    _connector = connector;
    _xeye_config = xeye_config;
    _input_queue = input_queue;
    _output_queue = output_queue;
    return 0;
}

void ConnectorMessageThread::uninit() {
    stop();
    _input_queue = NULL;
    _output_queue = NULL;
    _connector = NULL;
    _xeye_config = NULL;
}

int ConnectorMessageThread::start() {
    if (ConnectorMessageThread::_s_running) {
        printf("connector message thread is already running.\n");
        return -1;
    }

    ConnectorMessageThread::_s_running = true;
    _work_thread = new std::thread(ConnectorMessageThread::s_thread_func, \
                                   _connector, _input_queue, _output_queue, \
                                   _xeye_config);
    return 0;
}

int ConnectorMessageThread::stop() {
    if (!ConnectorMessageThread::_s_running) {
        printf("connector message thread is not running.\n");
        return -1;
    }

    if (NULL != _work_thread) {
        ConnectorMessageThread::_s_running = false;
        _work_thread->join();
        delete _work_thread;
        _work_thread = NULL;
    }
    return 0;
}

void ConnectorMessageThread::s_thread_func(UsbConnector *connector, \
                                           DlInputQueue *input_queue, \
                                           DlOutputQueue *output_queue, \
                                           XeyeConfig *xeye_config) {
    assert(NULL != connector);
    assert(NULL != xeye_config);
    assert(NULL != input_queue);
    assert(NULL != output_queue);

    while (ConnectorMessageThread::_s_running) {
        XeyeDataItem data_item;
        uint8_t *message_buffer = NULL;
        uint8_t *big_result_buffer = NULL;
        uint8_t *image_buffer = NULL;
        int ret = 0;

        do {
#ifdef USE_XEYE_BIG_BUF
            uint32_t message_buffer_size = XEYE_BIG_BUF_SIZE;
#else
            uint32_t message_buffer_size = XEYE_MESSAGE_SIZE;
#endif
            message_buffer = (uint8_t *)malloc(message_buffer_size);
            if (NULL == message_buffer) {
                printf("failed to malloc message_buf.\n");
                ret = -1;
                break;
            }
            memset(message_buffer, 0, XEYE_MESSAGE_SIZE);
            data_item.xeye_message = (XeyeMessage *)message_buffer;

            uint32_t timeout_ms = XEYE_VSC_TIMEOUT;
            static bool first_message = true;
            if (first_message) {
                timeout_ms = 0;
                first_message = false;
            }
            //printf("######## ConnectorMessageThread before receive message.\n");
            int ret = connector->receive(message_buffer, XEYE_MESSAGE_SIZE, \
                                         timeout_ms);
            //printf("######## ConnectorMessageThread after receive message.\n");
            if (0 == ret) {
                DEBUG_PRINT("receive successfully!\n");
            } else if (1 == ret) {
                DEBUG_PRINT("receive timeout to have chance to exit thread.\n");
                ret = -2;
                break;
            } else {
                printf("warning: failed to receive. ret:%d\n", ret);
                ret = -3;
                usleep(10000);
                break;
            }

            XeyeMessage &xeye_message = *(XeyeMessage *)message_buffer;
            DEBUG_PRINT("magic:0x%04x\n", xeye_message.magic);
            if (xeye_message.magic != XEYE_MAGIC) {
                printf("**** magic does not match. 0x%04x\n", \
                       xeye_message.magic);
                ret = -4;
                break;
            }

            switch (xeye_message.message_type) {
            case MESSAGETYPE_SET_OUTPUT:
            {
                if (xeye_message.with_result != 1) {
                    printf("MESSAGETYPE_SET_OUTPUT without result.\n");
                    ret = -5;
                    break;
                }

                XeyeImage &xeye_image = data_item.xeye_image;
                ImageInfo &image_info = xeye_message.image_info[0];
                ResultInfo &result_info = xeye_message.result_info;
                xeye_image.timestamp = image_info.timestamp;
                memcpy(xeye_image.extra, image_info.extra, \
                       EXTRA_IMAGE_INFO_SIZE);

                if (XEYEMODE_STANDARD == xeye_config->xeye_mode) {
                    xeye_image.org_height = image_info.height;
                    xeye_image.org_width = image_info.width;

                    xeye_image.org_image_type = image_info.image_type;
                    xeye_image.org_image_data_type = image_info.image_data_type;
                    assert(image_info.size == image_info.height * \
                           image_info.width * 3);
                    assert(IMAGETYPE_RRGGBB == image_info.image_type);
                    assert(IMAGEDATATYPE_UINT8 == image_info.image_data_type);
//                    assert(1 == xeye_message.image_count);
                }
                if (XEYEMODE_COMPUTATION == xeye_config->xeye_mode) {
                    xeye_image.height = image_info.height;
                    xeye_image.width = image_info.width;

                    xeye_image.image_type = image_info.image_type;
                    xeye_image.image_data_type = image_info.image_data_type;
                    assert(image_info.size == image_info.height * \
                           image_info.width * 3 * sizeof(uint16_t));
                    assert(IMAGETYPE_RGBRGB == image_info.image_type);
                    assert(IMAGEDATATYPE_FP16 == image_info.image_data_type);
//                    assert(0 == xeye_message.image_count);
                }

                if (0 == result_info.embedded) {
#ifdef USE_XEYE_BIG_BUF
                    uint32_t big_result_buffer_size = XEYE_BIG_BUF_SIZE;
#else
                    uint32_t big_result_buffer_size = result_info.result_size;
#endif
                    big_result_buffer = (uint8_t *)malloc(big_result_buffer_size);
                    if (NULL == big_result_buffer) {
                        printf("failed to malloc big_result_buffer.\n");
                        ret = -6;
                        break;
                    }

                    //receive big_result
                    //printf("######## ConnectorMessageThread before receive big_result.\n");
                    ret = connector->receive(big_result_buffer, result_info.result_size, \
                                             XEYE_VSC_TIMEOUT);
                    //printf("######## ConnectorMessageThread after receive big_result.\n");
                    if (0 == ret) {
                        DEBUG_PRINT("big_result receive successfully!\n");
                        result_info.big_result_data = (uint64_t)big_result_buffer;
                    } else if (1 == ret) {
                        DEBUG_PRINT("big_result receive timeout to have chance "\
                                    "to exit thread.\n");
                        ret = -7;
                        usleep(10000);
                        break;
                    } else {
                        printf("warning: failed to receive big_result. ret:%d\n", ret);
                        ret = -8;
                        break;
                    }
                }

                if (1 == xeye_message.image_count && \
                        1 == xeye_config->with_original_image) {
#ifdef USE_XEYE_BIG_BUF
                    uint32_t image_buffer_size = XEYE_BIG_BUF_SIZE;
#else
                    uint32_t image_buffer_size = image_info.size;
#endif
                    image_buffer = (uint8_t *)malloc(image_buffer_size);
                    if (NULL == image_buffer) {
                        printf("failed to malloc image_buffer.\n");
                        ret = -9;
                        break;
                    }

                    //receive image
                    //printf("######## ConnectorMessageThread before receive image.\n");
                    ret = connector->receive(image_buffer, image_info.size, \
                                             XEYE_VSC_TIMEOUT);
                    //printf("######## ConnectorMessageThread after receive image.\n");
                    if (0 == ret) {
                        DEBUG_PRINT("image receive successfully!\n");
                        data_item.xeye_image.org_image = image_buffer;
                    } else if (1 == ret) {
                        DEBUG_PRINT("image receive timeout to have chance "\
                                    "to exit thread.\n");
                        ret = -10;
                        usleep(10000);
                        break;
                    } else {
                        printf("warning: failed to receive. ret:%d\n", ret);
                        ret = -11;
                        break;
                    }
                }

                //push data_item to output_queue
                ret = output_queue->push(data_item);
                if (0 != ret) {
                    printf("output_queue is full. delete the oldest item.\n");
                    XeyeDataItem item;
                    output_queue->pop(item);

                    free((void *)item.xeye_message->result_info.big_result_data);
                    item.xeye_message->result_info.big_result_data = 0;
                    free(item.xeye_message);
                    item.xeye_message = NULL;
                    delete item.xeye_result;
                    item.xeye_result = NULL;
                    free(item.xeye_image.org_image);
                    item.xeye_image.org_image = NULL;
                    assert(NULL == item.xeye_image.image);

                    ret = output_queue->push(data_item);
                    assert(0 == ret);
                }
                ret = 0;
                break;
            }
            default:
                printf("host cannot handle message type %d.\n",
                       xeye_message.message_type);
                ret = -12;
                break;
            }
        } while (false);

        if (0 != ret) {
            printf("warning: failed to push output_queue. free all.\n");
            free(message_buffer);
            message_buffer = NULL;
            free(big_result_buffer);
            big_result_buffer = NULL;
            free(image_buffer);
            image_buffer = NULL;
        }
        //else do not forget to free message_buffer and image_buffer in future
    }
}
