/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_XEYE_DATA_TYPE_H
#define BAIDU_XEYE_XEYE_DATA_TYPE_H

#include "xeye_message_data_type.h"

#include <opencv2/opencv.hpp>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USE_XEYE_BIG_BUF

#define XEYE_MODEL_SIZE (15 * 1024 * 1024)
#define XEYE_MESSAGE_DATA_SIZE (8 * 1024 * 1024)
#define XEYE_BIG_BUF_SIZE XEYE_MESSAGE_DATA_SIZE
//#define XEYE_VSC_TIMEOUT (3 * 1000)
#define XEYE_VSC_TIMEOUT (0)

struct XeyeConfig {
    int xeye_mode;
    int enable_preprocess;
    int enable_resize;
    uint32_t resize_height;
    uint32_t resize_width;
    int enable_mean;
    float mean_b;
    float mean_g;
    float mean_r;
    int enable_scale;
    float scale;
    int with_original_image;

    XeyeConfig() {
        xeye_mode = 0;
        enable_preprocess = 0;
        enable_resize = 0;
        resize_height = 0;
        resize_width = 0;
        enable_mean = 0;
        mean_b = 0.0;
        mean_g = 0.0;
        mean_r = 0.0;
        enable_scale = 0;
        scale = 0.0;
        with_original_image = 0;
    }
};

struct ModelData {
    uint32_t index;
    char model_name[16];
    uint8_t *model_data;
    uint32_t model_size;

    ModelData() {
        index = 0;
        memset(model_name, 0, sizeof(model_name));
        model_data = NULL;
        model_size = 0;
    }
};

struct XeyeImage {
    uint64_t timestamp; //image's capture timestamp
    uint32_t org_image_type; //ImageType
    uint32_t org_image_data_type; //ImageDataType
    uint32_t image_type; //ImageType
    uint32_t image_data_type; //ImageDataType
    uint8_t extra[EXTRA_IMAGE_INFO_SIZE]; //for example camera_id, device_id
    uint32_t org_height;
    uint32_t org_width;
    uint8_t *org_image;
    cv::Mat org_mat;
    uint32_t height; //after proprocessed
    uint32_t width; //after proprocessed
    uint16_t *image; //after proprocessed
    float *float_image; //after proprocessed
    cv::Mat mat; //after proprocessed

    XeyeImage() {
        timestamp = 0;
        org_image_type = IMAGETYPE_RRGGBB;
        org_image_data_type = IMAGEDATATYPE_FP16;
        image_type = IMAGETYPE_RRGGBB;
        image_data_type = IMAGEDATATYPE_FP16;
        memset(extra, 0, sizeof(extra));
        org_height = 0;
        org_width = 0;
        org_image = NULL;
        height = 0;
        width = 0;
        image = NULL;
        float_image = NULL;
    }
};

struct XeyeResult {
    int embedded;
    uint32_t result_size; //total result size
    uint8_t result_data[RESULT_DATA_SIZE * 2]; //all results store here
    uint8_t *big_result_data;

    XeyeResult() {
        embedded = 0;
        result_size = 0;
        memset(result_data, 0, sizeof(result_data));
        big_result_data = NULL;
    }
};

struct XeyeDataItem {
    XeyeMessage *xeye_message;
    XeyeImage xeye_image;
    XeyeResult *xeye_result;

    XeyeDataItem() {
        xeye_message = NULL;
        xeye_result = NULL;
    }
};

typedef void (* XEYE_OUTPUT_HANDLER)(XeyeDataItem *data_item);
typedef void (* XEYE_WATCHDOG_HANDLER)();

#endif //BAIDU_XEYE_XEYE_DATA_TYPE_H
