/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "dl_output_thread.h"
#include "dl_process.h"
#include "debug_def.h"

#include <unistd.h>
#include <assert.h>

bool DlOutputThread::_s_running = false;

DlOutputThread::DlOutputThread() {
    _output_queue = NULL;
    _work_thread = NULL;
    _output_handler = NULL;
    _watchdog_handler = NULL;
    _xeye_config = NULL;
    DlOutputThread::_s_running = false;
}

DlOutputThread::~DlOutputThread() {
    uninit();
}

int DlOutputThread::init(DlOutputQueue *output_queue, \
                         XEYE_OUTPUT_HANDLER output_handler, \
                         XEYE_WATCHDOG_HANDLER watchdog_handler, \
                         XeyeConfig *config) {
    if (NULL == output_queue || NULL == output_handler || \
        NULL == watchdog_handler || NULL == config) {
        printf("DlOutputThread::init invalid parameters.\n");
        return -1;
    }
    _output_queue = output_queue;
    _output_handler = output_handler;
    _watchdog_handler = watchdog_handler;
    _xeye_config = config;
    return 0;
}

void DlOutputThread::uninit() {
    stop();
    _output_queue = NULL;
    _output_handler = NULL;
    _watchdog_handler = NULL;
    _xeye_config = NULL;
}

int DlOutputThread::start() {
    if (DlOutputThread::_s_running) {
        printf("dl output thread is already running.\n");
        return -1;
    }

    DlOutputThread::_s_running = true;
    _work_thread = new std::thread(DlOutputThread::s_thread_func, \
                                   _output_queue, _output_handler, \
                                   _watchdog_handler, _xeye_config);
    return 0;
}

int DlOutputThread::stop() {
    if (!DlOutputThread::_s_running) {
        printf("dl output thread is not running.\n");
        return -1;
    }

    if (NULL != _work_thread) {
        DlOutputThread::_s_running = false;
        _work_thread->join();
        delete _work_thread;
        _work_thread = NULL;
    }
    return 0;
}

void DlOutputThread::s_thread_func(DlOutputQueue *output_queue, \
                                   XEYE_OUTPUT_HANDLER output_handler, \
                                   XEYE_WATCHDOG_HANDLER watchdog_handler, \
                                   XeyeConfig *config) {
    assert(NULL != output_queue);
    assert(NULL != output_handler);
    assert(NULL != watchdog_handler);
    assert(NULL != config);

    uint32_t empty_count = 0;
    while (DlOutputThread::_s_running) {
        XeyeDataItem data_item;
        int ret = output_queue->pop(data_item);
        if (0 != ret) {
            //printf("output_queue is empty.\n");
            usleep(10000);
            if (0 == ++empty_count % 1600) { //reset after 16s block
                watchdog_handler();
            }
            continue;
        }
        empty_count = 0;

        ret = DlProcess::postprocess(*config, data_item);
        if (0 == ret) {
            output_handler(&data_item);
        } else {
            printf("postprocess failed. ret:%d\n", ret);
        }

        free((void *)data_item.xeye_message->result_info.big_result_data);
        data_item.xeye_message->result_info.big_result_data = 0;
        free(data_item.xeye_message);
        data_item.xeye_message = NULL;
        free(data_item.xeye_result->big_result_data);
        data_item.xeye_result->big_result_data = NULL;
        delete data_item.xeye_result;
        data_item.xeye_result = NULL;
        free(data_item.xeye_image.org_image);
        data_item.xeye_image.org_image = NULL;
        assert(NULL == data_item.xeye_image.image);
    }
}
