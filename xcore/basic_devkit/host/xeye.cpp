/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "xeye.h"
#include "xeye_config.h"
#include "xeye_util.h"
#include "debug_def.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

Xeye::Xeye() {
    _xeye_mode = XEYEMODE_UNKNOWN;
    _connector = NULL;
    _model_file_map = NULL;
    _config_file_map = NULL;
    _xeye_config = NULL;
    _message_thread = NULL;
    _input_queue = NULL;
    _input_thread = NULL;
    _output_queue = NULL;
    _output_thread = NULL;
}

Xeye::~Xeye() {
    uninit();
}

int Xeye::init(const char* model_file_path, const char* config_file_path,
               XEYE_OUTPUT_HANDLER output_handler, \
               XEYE_WATCHDOG_HANDLER watchdog_handler, int mode) {
    if (NULL == model_file_path || NULL == config_file_path || \
            NULL == output_handler || NULL == watchdog_handler || \
            mode <= XEYEMODE_UNKNOWN || mode >= XEYEMODE_MAX) {
        printf("Xeye::init failed. invalid parameters.\n");
        return -1;
    }
    assert(sizeof(XeyeMessage) <= XEYE_MESSAGE_SIZE);

    int ret = 0;
    int max_input_queue_size = 4;
    int max_output_queue_size = 4;

    do {
        _xeye_mode = (XeyeMode)mode;

        _connector = new UsbConnector;
        ret = _connector->init();
        if (0 != ret) {
            printf("connection(linux->xeye) init failed. ret:%d\n", ret);
            ret = -2;
            break;
        }

        _model_file_map = new FileMap;
        ret = _model_file_map->init(model_file_path);
        if (0 != ret) {
            printf("failed to create model file map %s.\n", model_file_path);
            ret = -3;
            break;
        }

        _config_file_map = new FileMap;
        ret = _config_file_map->init(config_file_path);
        if (0 != ret) {
            printf("failed to create config file map %s.\n", config_file_path);
            ret = -4;
            break;
        }

        _xeye_config = new XeyeConfig;
        _xeye_config->xeye_mode = _xeye_mode;
        ret = parse_config_file(_config_file_map, _xeye_config);
        if (0 != ret) {
            printf("failed to parse json config file %s.\n", config_file_path);
            ret = -5;
            break;
        }

        _input_queue = new DlInputQueue(max_input_queue_size);
        _output_queue = new DlOutputQueue(max_output_queue_size);

        _message_thread = new ConnectorMessageThread;
        ret = _message_thread->init(_connector, _input_queue, _output_queue, \
                                    _xeye_config);
        if (0 != ret) {
            printf("failed to init message thread. ret:%d\n", ret);
            ret = -6;
            break;
        }

        _input_thread = new DlInputThread;
        ret = _input_thread->init(_connector, _input_queue, _xeye_config);
        if (0 != ret) {
            printf("failed to init input thread. ret:%d\n", ret);
            ret = -7;
            break;
        }

        _output_thread = new DlOutputThread;
        ret = _output_thread->init(_output_queue, output_handler, \
                                   watchdog_handler, _xeye_config);
        if (0 != ret) {
            printf("failed to init output thread. ret:%d\n", ret);
            ret = -8;
            break;
        }

        ret = _message_thread->start();
        if (0 != ret) {
            printf("failed to start message thread. ret:%d\n", ret);
            ret = -9;
            break;
        }

        ret = _input_thread->start();
        if (0 != ret) {
            printf("failed to start input thread. ret:%d\n", ret);
            ret = -10;
            break;
        }

        ret = _output_thread->start();
        if (0 != ret) {
            printf("failed to start output thread. ret:%d\n", ret);
            ret = -11;
            break;
        }
    } while (false);

    if (0 != ret) {
        uninit();
    }
    return ret;
}

int Xeye::start() {
    if (NULL == _connector || NULL == _model_file_map || \
            NULL == _config_file_map || NULL == _xeye_config || \
            NULL == _message_thread || NULL == _input_queue || \
            NULL == _input_thread || NULL == _output_queue || \
            NULL == _output_thread) {
        printf("you must init xeye at first!\n");
        return -1;
    }
    if (_xeye_mode <= XEYEMODE_UNKNOWN || _xeye_mode >= XEYEMODE_MAX) {
        printf("invalid xeye_mode:%d\n", _xeye_mode);
        return -2;
    }

    int ret = 0;
    uint8_t *message_buffer = NULL;
    do {
#ifdef USE_XEYE_BIG_BUF
        uint32_t message_buffer_size = XEYE_BIG_BUF_SIZE;
#else
        uint32_t message_buffer_size = XEYE_MESSAGE_SIZE;
#endif
        message_buffer = (uint8_t *)malloc(message_buffer_size);
        if (NULL == message_buffer) {
            printf("failed to malloc message_buf.\n");
            ret = -3;
            break;
        }
        memset(message_buffer, 0, XEYE_MESSAGE_SIZE);

        XeyeMessage &xeye_message = *(XeyeMessage *)message_buffer;
        xeye_message.magic = XEYE_MAGIC;
        xeye_message.sender = CONNECTTARGET_HOST;
        xeye_message.receiver = CONNECTTARGET_XEYE;
        xeye_message.message_type = MESSAGETYPE_CONFIG_XEYE;
        xeye_message.with_config = 1;
        xeye_message.model_count = 1;
        xeye_message.image_count = 0;
        xeye_message.with_result = 0;

        ConfigInfo &config_info = xeye_message.config_info;
        config_info.xeye_mode = _xeye_mode;
        config_info.config_file_size = _config_file_map->size();
        printf("$$$$ config_file_size:%u\n", config_info.config_file_size);
        if (config_info.config_file_size > CONFIG_FILE_SIZE) {
            printf("config file is too large %d > %d\n",
                   config_info.config_file_size, CONFIG_FILE_SIZE);
            ret = -4;
            break;
        }
        memcpy(config_info.config_file_data, _config_file_map->map(), \
               config_info.config_file_size);

        ModelInfo &model_info = xeye_message.model_info[0];
        snprintf((char *)model_info.model_name, 16, "%s", "graph");
        model_info.model_file_size = _model_file_map->size();

        //send MESSAGETYPE_CONFIG_XEYE
        //printf("######## start before send message.\n");
        ret = _connector->send(message_buffer, XEYE_MESSAGE_SIZE, \
                               XEYE_VSC_TIMEOUT);
        //printf("######## start after send message.\n");
        if (0 == ret) {
            DEBUG_PRINT("send MESSAGETYPE_CONFIG_XEYE successfully!\n");
        } else if (1 == ret) {
            DEBUG_PRINT("send MESSAGETYPE_CONFIG_XEYE timeout.\n");
            ret = -5;
            break;
        } else {
            printf("warning: failed to send MESSAGETYPE_CONFIG_XEYE. "\
                   "ret:%d\n", ret);
            ret = -6;
            break;
        }

        //send model to xeye
        //printf("######## start before send model.\n");
        ret = _connector->send((uint8_t *)_model_file_map->map(), \
                               _model_file_map->size(), XEYE_VSC_TIMEOUT);
        //printf("######## start after send model.\n");
        if (0 == ret) {
            DEBUG_PRINT("send model successfully!\n");
        } else if (1 == ret) {
            DEBUG_PRINT("send model timeout.\n");
            ret = -7;
            break;
        } else {
            printf("warning: failed to send model. ret:%d\n", \
                   ret);
            ret = -8;
            break;
        }
    } while (false);

    free(message_buffer);
    message_buffer = NULL;
    return ret;
}

int Xeye::stop() {
    if (NULL == _connector || NULL == _model_file_map || \
            NULL == _config_file_map || NULL == _xeye_config || \
            NULL == _message_thread || NULL == _input_queue || \
            NULL == _input_thread || NULL == _output_queue || \
            NULL == _output_thread) {
        printf("you must init xeye at first!\n");
        return -1;
    }
    if (_xeye_mode <= XEYEMODE_UNKNOWN || _xeye_mode >= XEYEMODE_MAX) {
        printf("invalid xeye_mode:%d\n", _xeye_mode);
        return -2;
    }
    if (XEYEMODE_STANDARD == _xeye_mode) {
        printf("in standard mode, no need to send stop message to device.\n");
        return 0;
    }

    int ret = 0;
    uint8_t *message_buffer = NULL;
    do {
        message_buffer = (uint8_t *)malloc(XEYE_MESSAGE_SIZE);
        if (NULL == message_buffer) {
            printf("failed to malloc message_buf.\n");
            ret = -3;
            break;
        }
        memset(message_buffer, 0, XEYE_MESSAGE_SIZE);

        XeyeMessage &xeye_message = *(XeyeMessage *)message_buffer;
        xeye_message.magic = XEYE_MAGIC;
        xeye_message.sender = CONNECTTARGET_HOST;
        xeye_message.receiver = CONNECTTARGET_XEYE;
        xeye_message.message_type = MESSAGETYPE_STOP_XEYE;
        xeye_message.with_config = 0;
        xeye_message.model_count = 0;
        xeye_message.image_count = 0;
        xeye_message.with_result = 0;

        //send MESSAGETYPE_CONFIG_XEYE
        ret = _connector->send(message_buffer, XEYE_MESSAGE_SIZE, \
                               XEYE_VSC_TIMEOUT);
        if (0 == ret) {
            DEBUG_PRINT("send MESSAGETYPE_STOP_XEYE successfully!\n");
        } else if (1 == ret) {
            DEBUG_PRINT("send MESSAGETYPE_STOP_XEYE timeout.\n");
            ret = -4;
            break;
        } else {
            printf("warning: failed to send MESSAGETYPE_STOP_XEYE. ret:%d\n", \
                   ret);
            ret = -5;
            break;
        }
    } while (false);

    free(message_buffer);
    message_buffer = NULL;
    return ret;
}

void Xeye::uninit() {
    if (NULL != _message_thread) {
        _message_thread->stop();
        delete _message_thread;
        _message_thread = NULL;
    }

    if (NULL != _input_thread) {
        _input_thread->stop();
        delete _input_thread;
        _input_thread = NULL;
    }

    if (NULL != _output_thread) {
        _output_thread->stop();
        delete _output_thread;
        _output_thread = NULL;
    }

    delete _input_queue;
    _input_queue = NULL;
    delete _output_queue;
    _output_queue = NULL;

    delete _config_file_map;
    _config_file_map = NULL;
    delete _model_file_map;
    _model_file_map = NULL;
    delete _connector;
    _connector = NULL;

    _xeye_mode = XEYEMODE_UNKNOWN;
}
