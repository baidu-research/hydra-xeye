/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_CONNECTOR_MESSAGE_THREAD_H
#define BAIDU_XEYE_CONNECTOR_MESSAGE_THREAD_H

#include "usb_connector.h"
#include "dl_input_thread.h"
#include "dl_output_thread.h"

#include <thread>

class ConnectorMessageThread {
public:
    ConnectorMessageThread();
    ~ConnectorMessageThread();

    int init(UsbConnector *connector, DlInputQueue *input_queue, \
             DlOutputQueue *output_queue, XeyeConfig *xeye_config);
    int start();
    int stop();

private:
    void uninit();
    static void s_thread_func(UsbConnector *connector, \
                              DlInputQueue *input_queue, \
                              DlOutputQueue *output_queue, \
                              XeyeConfig *xeye_config);

    static bool _s_running;
    DlInputQueue *_input_queue;
    DlOutputQueue *_output_queue;
    UsbConnector *_connector;
    XeyeConfig *_xeye_config;
    std::thread *_work_thread;
};

#endif // BAIDU_XEYE_CONNECTOR_MESSAGE_THREAD_H
