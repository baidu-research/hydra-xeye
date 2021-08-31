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

#include "vsc/xeye_message_data_type.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USE_XEYE_BIG_BUF

#define XEYE_MODEL_SIZE (20 * 1024 * 1024)
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
    uint32_t org_height;
    uint32_t org_width;
    uint8_t *org_image;
    uint32_t height; //after proprocessed
    uint32_t width; //after proprocessed
    uint16_t *image; //after proprocessed

    XeyeImage() {
        org_height = 0;
        org_width = 0;
        org_image = NULL;
        height = 0;
        width = 0;
        image = NULL;
    }
};

extern struct XeyeImage lrt_g_xeye_input;
extern struct XeyeConfig lrt_g_xeye_config;
extern struct XeyeMessage *lrt_g_xeye_input_message;

#endif //BAIDU_XEYE_XEYE_DATA_TYPE_H
