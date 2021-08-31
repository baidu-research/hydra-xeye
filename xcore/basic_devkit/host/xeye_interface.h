/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_XEYE_INTERFACE_H
#define BAIDU_XEYE_XEYE_INTERFACE_H

#include "xeye_data_type.h"

//Xeye with camera
class IXeye {
public:
    /*
     * open usb connection between PC/ARM and Xeye, start and stop method 
     * will use this connection internally.
     * listen thread listen on the connection to accept output.
     * model_file_path: graph file path
     * config_file_path: json file to config preprocess
     * output_handler: handler to process deep learning result from Xeye
     * watchdog_handler: handler to reboot if block for 6 seconds or more
     * mode: 0 - unknown mode, 1 - standard mode, 2 - computation mode
     * return value: 0 - succeed, other - failed
     */
    virtual int init(const char* model_file_path, \
                     const char* config_file_path, 
                     XEYE_OUTPUT_HANDLER output_handler, \
                     XEYE_WATCHDOG_HANDLER watchdog_handler, \
                     int mode) = 0;

    /*
     * send message MESSAGETYPE_CONFIG_XEYE to Xeye.
     * return value: 0 - succeed, other - failed
     */
    virtual int start() = 0;

    /*
     * send message MESSAGETYPE_STOP_XEYE to Xeye.
     * stop deep learning on Xeye, Xeye's mode reset to unknown mode,
     * then Xeye will keep on listening on connection to accept 
     * MESSAGETYPE_CONFIG_XEYE message.
     * return value: 0 - succeed, other - failed
     */
    virtual int stop() = 0;

    /*
     * close usb connection, exit listen thread.
     */
    virtual void uninit() = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

int create_xeye_instance(const char* model_file_path,
                         const char* config_file_path,
                         XEYE_OUTPUT_HANDLER output_handler,
                         XEYE_WATCHDOG_HANDLER watchdog_handler,
                         IXeye **xeye);
void destroy_xeye_instance(IXeye *xeye);

#ifdef __cplusplus
}
#endif

#endif // BAIDU_XEYE_XEYE_INTERFACE_H
