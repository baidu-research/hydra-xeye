/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_XEYE_COMPUTATION_INTERFACE_H
#define BAIDU_XEYE_XEYE_COMPUTATION_INTERFACE_H

#include "xeye_interface.h"

#include <string.h>

struct XeyeInputContext {
    uint64_t timestamp;
    uint32_t image_height;
    uint32_t image_width;
    uint8_t extra[EXTRA_IMAGE_INFO_SIZE]; //for example camera_id, device_id

    XeyeInputContext() {
        timestamp = 0;
        image_height = 0;
        image_width = 0;
        memset(extra, 0, EXTRA_IMAGE_INFO_SIZE);
    }
};

//Xeye without camera
class IXeyeComputation : public IXeye {
public:
    /*
     * push image to work queue, then host will do the proprocess before
     * sending XeyeInput to Xeye.
     * return value: 0 - succeed, other - failed
     */
    virtual int push_image(XeyeInputContext *input_context, \
                           const char *image_file_path) = 0;

    /*
     * push image opencv matrix to work queue,  then host will do the
     * proprocess before sending XeyeInput to Xeye.
     * return value: 0 - succeed, other - failed
     */
    virtual int push_image(XeyeInputContext *input_context, void* mat) = 0;

    /*
     * push image(pixel of bgr channel order) before preprocess to work queue.
     * return value: 0 - succeed, other - failed
     */
    virtual int push_image(XeyeInputContext *input_context, \
                           uint8_t* image_buf, uint32_t image_buf_size) = 0;

    /*
     * push image preprocessed to work queue.
     * in json config file, enable_preprocess must be 0
     * return value: 0 - succeed, other - failed
     */
    virtual int push_image(XeyeInputContext *input_context, \
                           float* image_buf, uint32_t image_buf_size) = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

int create_xeye_computation_instance(const char* model_file_path,
                                     const char* config_file_path,
                                     XEYE_OUTPUT_HANDLER output_handler,
                                     XEYE_WATCHDOG_HANDLER watchdog_handler,
                                     IXeyeComputation **xeye_computation);
int destroy_xeye_computation_instance(IXeyeComputation *xeye_computation);

#ifdef __cplusplus
}
#endif

#endif // BAIDU_XEYE_XEYE_COMPUTATION_INTERFACE_H
