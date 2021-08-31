/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_DL_INPUT_THREAD_H
#define BAIDU_XEYE_DL_INPUT_THREAD_H

#include "xeye_data_type.h"
#include "xeye_message_data_type.h"
#include "usb_connector.h"
#include "dl_queue.h"

#include <thread>

typedef DlQueue<XeyeDataItem> DlInputQueue;

class DlInputThread {
public:
    DlInputThread();
    ~DlInputThread();

    int init(UsbConnector *connector, DlInputQueue *input_queue, \
             XeyeConfig *config);
    int start();
    int stop();

private:
    void uninit();
    static void s_thread_func(UsbConnector *connector, \
                              DlInputQueue *input_queue, XeyeConfig *config);

    static bool _s_running;
    XeyeConfig *_xeye_config;
    DlInputQueue *_input_queue;
    UsbConnector *_connector;
    std::thread *_work_thread;
};

#endif // BAIDU_XEYE_DL_INPUT_THREAD_H
