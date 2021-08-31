/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_XEYE_COMPUTATION_H
#define BAIDU_XEYE_XEYE_COMPUTATION_H

#include "xeye_computation_interface.h"
#include "xeye.h"
#include "xeye_message_data_type.h"

class XeyeComputation : public IXeyeComputation {
public:
    XeyeComputation();
    ~XeyeComputation();

    /*
     * run under mode XM_XEYE_COMPUTATION (without camera).
     * model_file_path: graph file path
     * config_file_path: json file to config preprocess run on Linux
     * handler: handler to process deep learning result with image from Xeye
     */
    int init(const char* model_file_path, const char* config_file_path, \
             XEYE_OUTPUT_HANDLER output_handler, \
             XEYE_WATCHDOG_HANDLER watchdog_handler, \
             int mode = XEYEMODE_COMPUTATION);

    int start();
    int stop();
    void uninit();

    int push_image(XeyeInputContext *input_context, \
                   const char *image_file_path);
    int push_image(XeyeInputContext *input_context, void* mat);
    int push_image(XeyeInputContext *input_context, uint8_t* image_buf, \
                   uint32_t image_buf_size);
    int push_image(XeyeInputContext *input_context, float* image_buf, \
                   uint32_t image_buf_size);

    Xeye * xeye();

private:
    int push_data_item(XeyeDataItem &data_item);
    Xeye *_xeye;
};

#endif //BAIDU_XEYE_XEYE_COMPUTATION_H
