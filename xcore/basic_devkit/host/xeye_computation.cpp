/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "xeye_computation.h"
#include "xeye_util.h"
#include "debug_def.h"

#include <opencv2/opencv.hpp>

#include <assert.h>

XeyeComputation::XeyeComputation() {
    _xeye = NULL;
}

XeyeComputation::~XeyeComputation() {
    uninit();
}

int XeyeComputation::init(const char* model_file_path, \
                          const char* config_file_path, \
                          XEYE_OUTPUT_HANDLER output_handler, \
                          XEYE_WATCHDOG_HANDLER watchdog_handler, int mode) {
    if (NULL == model_file_path || NULL == config_file_path || \
            NULL == output_handler ||  NULL == watchdog_handler || \
            mode <= XEYEMODE_UNKNOWN || mode >= XEYEMODE_MAX) {
        printf("XeyeComputation::init failed. invalid parameters.\n");
        return -1;
    }

    _xeye = new Xeye;
    int ret = _xeye->init(model_file_path, config_file_path, output_handler, \
                          watchdog_handler, mode);
    if (0 != ret) {
        printf("XeyeComputation::init failed. ret:%d\n", ret);
    }
    return ret;
}

int XeyeComputation::start() {
    if (NULL == _xeye) {
        printf("you must init XeyeComputation at first!\n");
        return -1;
    }
    int ret = _xeye->start();
    if (0 != ret) {
        printf("XeyeComputation::start failed. ret:%d\n", ret);
    }
    return ret;
}

int XeyeComputation::stop() {
    if (NULL == _xeye) {
        printf("you must init XeyeComputation at first!\n");
        return -1;
    }
    int ret = _xeye->stop();
    if (0 != ret) {
        printf("XeyeComputation::stop failed. ret:%d\n", ret);
    }
    return ret;
}

void XeyeComputation::uninit() {
    delete (Xeye *)_xeye;
    _xeye = NULL;
}

int XeyeComputation::push_image(XeyeInputContext *input_context, \
                                const char *image_file_path) {
    if (NULL == image_file_path) {
        printf("image_file_path is NULL.\n");
        return -1;
    }

    cv::Mat mat = cv::imread(image_file_path);
    if (NULL == mat.data) {
        printf("imread %s failed.\n", image_file_path);
        return -2;
    }
    return push_image(input_context, &mat);
}

int XeyeComputation::push_image(XeyeInputContext *input_context, void* mat) {
    if (NULL == mat) {
        printf("push_image mat is NULL.\n");
        return -1;
    }
    if (NULL == _xeye || NULL == _xeye->_input_queue) {
        printf("invalid _xeye.\n");
        return -2;
    }

    cv::Mat &mat_ref = *(cv::Mat *)mat;
    XeyeDataItem data_item;
    XeyeImage &xeye_image = data_item.xeye_image;
    xeye_image.org_image_type = IMAGETYPE_BGRBGR;
    xeye_image.org_image_data_type = IMAGEDATATYPE_UINT8;
    xeye_image.image_type = IMAGETYPE_RGBRGB;
    xeye_image.image_data_type = IMAGEDATATYPE_FP16;
    if (NULL == input_context) {
        xeye_image.timestamp = XeyeUtil::unix_timestamp();
    } else {
        xeye_image.timestamp = input_context->timestamp;
        memcpy(xeye_image.extra, input_context->extra, EXTRA_IMAGE_INFO_SIZE);
    }
    xeye_image.org_height = mat_ref.rows;
    xeye_image.org_width = mat_ref.cols;
    xeye_image.org_image = (uint8_t *)mat_ref.datastart;
    xeye_image.org_mat = mat_ref;

    return push_data_item(data_item);
}

int XeyeComputation::push_image(XeyeInputContext *input_context, \
                                uint8_t* image_buf, uint32_t image_buf_size) {
    if (NULL == input_context || NULL == image_buf || 0 == image_buf_size) {
        printf("invalid parameters.\n");
        return -1;
    }

    XeyeDataItem data_item;
    XeyeImage &xeye_image = data_item.xeye_image;
    xeye_image.org_image_type = IMAGETYPE_BGRBGR;
    xeye_image.org_image_data_type = IMAGEDATATYPE_UINT8;
    xeye_image.image_type = IMAGETYPE_RGBRGB;
    xeye_image.image_data_type = IMAGEDATATYPE_FP16;
    if (NULL == input_context) {
        xeye_image.timestamp = XeyeUtil::unix_timestamp();
    } else {
        xeye_image.timestamp = input_context->timestamp;
        memcpy(xeye_image.extra, input_context->extra, EXTRA_IMAGE_INFO_SIZE);
    }
    xeye_image.org_height = input_context->image_height;
    xeye_image.org_width = input_context->image_width;
    xeye_image.org_image = image_buf;

    // if (image_buf_size != xeye_image.org_height * xeye_image.org_width * 3) {
    //     printf("height/width in input_context mismatch "\
    //            "image_buf_size(uint8_t).\n");
    //     printf("h:%u w:%u h*w*3:%u image_buf_size:%u\n", \
    //            xeye_image.org_height, xeye_image.org_width, \
    //            xeye_image.org_height * xeye_image.org_width * 3, image_buf_size);
    //     return -2;
    // }
    return push_data_item(data_item);
}

int XeyeComputation::push_image(XeyeInputContext *input_context, \
                                float* image_buf, uint32_t image_buf_size) {
    if (NULL == input_context || NULL == image_buf || 0 == image_buf_size) {
        printf("invalid parameters.\n");
        return -1;
    }

    if (0 != _xeye->_xeye_config->enable_preprocess) {
        printf("preprocess in config must be disabled.\n");
        return -2;
    }
    XeyeDataItem data_item;
    XeyeImage &xeye_image = data_item.xeye_image;
    xeye_image.image_type = IMAGETYPE_RGBRGB;
    xeye_image.image_data_type = IMAGEDATATYPE_FP16;
    if (NULL == input_context) {
        xeye_image.timestamp = XeyeUtil::unix_timestamp();
    } else {
        xeye_image.timestamp = input_context->timestamp;
        memcpy(xeye_image.extra, input_context->extra, EXTRA_IMAGE_INFO_SIZE);
    }
    xeye_image.height = input_context->image_height;
    xeye_image.width = input_context->image_width;
    xeye_image.float_image = image_buf;

    if (image_buf_size != xeye_image.height * xeye_image.width * \
            3 * sizeof(float)) {
        printf("height/width in input_context mismatch "\
               "image_buf_size(float).\n");
        return -2;
    }
    return push_data_item(data_item);
}

int XeyeComputation::push_data_item(XeyeDataItem &data_item) {
    int ret = _xeye->_input_queue->push(data_item);
    if (-1 == ret) {
        XeyeDataItem item;
        ret = _xeye->_input_queue->pop(item);
        if (0 == ret) {
            XeyeImage &xeye_image = item.xeye_image;
            if (NULL != xeye_image.org_mat.data) {
                xeye_image.org_mat.release();
                xeye_image.org_image = NULL;
            } else if (NULL != xeye_image.org_image) {
                free(xeye_image.org_image);
                xeye_image.org_image = NULL;
            } else if (NULL != xeye_image.float_image) {
                free(xeye_image.float_image);
                xeye_image.float_image = NULL;
            } else {
                printf("warning: push_data_item should not execute here.\n");
            }
            assert(NULL == item.xeye_message);
            assert(NULL == item.xeye_result);
        }
        ret = _xeye->_input_queue->push(data_item);
        return ret;
    }
    return ret;
}

Xeye * XeyeComputation::xeye() {
    return _xeye;
}
