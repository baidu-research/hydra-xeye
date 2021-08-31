/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_DL_OUTPUT_THREAD_H
#define BAIDU_XEYE_DL_OUTPUT_THREAD_H

#include "usb_connector.h"
#include "xeye_data_type.h"
#include "xeye_message_data_type.h"
#include "dl_queue.h"

#include <thread>

typedef DlQueue<XeyeDataItem> DlOutputQueue;

class DlOutputThread {
public:
    DlOutputThread();
    ~DlOutputThread();

    int init(DlOutputQueue *output_queue, XEYE_OUTPUT_HANDLER output_handler, \
             XEYE_WATCHDOG_HANDLER watchdog_handler, XeyeConfig *config);
    int start();
    int stop();

private:
    void uninit();
    static void s_thread_func(DlOutputQueue *output_queue, \
                              XEYE_OUTPUT_HANDLER output_handler, \
                              XEYE_WATCHDOG_HANDLER watchdog_handler, \
                              XeyeConfig *config);

    static bool _s_running;
    DlOutputQueue *_output_queue;
    std::thread *_work_thread;
    XEYE_OUTPUT_HANDLER _output_handler;
    XEYE_WATCHDOG_HANDLER _watchdog_handler;
    XeyeConfig *_xeye_config;
};

#endif // BAIDU_XEYE_DL_OUTPUT_THREAD_H
