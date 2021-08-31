/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_XEYE_H
#define BAIDU_XEYE_XEYE_H

#include "xeye_interface.h"
#include "usb_connector.h"
#include "xeye_message_data_type.h"
#include "connector_message_thread.h"
#include "dl_input_thread.h"
#include "dl_output_thread.h"
#include "file_map.h"

class Xeye : public IXeye {
public:
    Xeye();
    ~Xeye();

    /*
     * run under mode XM_XEYE (with camera).
     * model_file_path: graph file path
     * config_file_path: json file to config preprocess on Linux
     * output_handler: handler to process deep learning result from Xeye
     * watchdog_handler: handler to reboot if block for 6 seconds or more
     */
    int init(const char* model_file_path, const char* config_file_path,
             XEYE_OUTPUT_HANDLER output_handler, \
             XEYE_WATCHDOG_HANDLER watchdog_handler, \
             int mode = XEYEMODE_STANDARD);

    int start();
    int stop();
    void uninit();

public:
    XeyeMode _xeye_mode;
    UsbConnector *_connector;
    FileMap *_model_file_map;
    FileMap *_config_file_map;
    XeyeConfig *_xeye_config;
    ConnectorMessageThread *_message_thread;
    DlInputQueue *_input_queue;
    DlInputThread *_input_thread;
    DlOutputQueue *_output_queue;
    DlOutputThread *_output_thread;
};

#endif //BAIDU_XEYE_XEYE_H
