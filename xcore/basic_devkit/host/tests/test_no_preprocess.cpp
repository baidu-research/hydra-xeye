/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "xeye_computation_interface.h"
#include "xeye_util.h"
#include "ssd_output_handler.h"
#include "watchdog_handler.h"
#include "dl_process.h"
#include "xeye_computation.h"
#include "fp16_convert.h"

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <opencv2/opencv.hpp>

#include <atomic>
#include <string>

struct CameraInfo {
    char device_id[16];
    char camera_id[16];
};

std::atomic<bool> g_run_flag;

void sig_hanlder(int sig_type) {
    if (sig_type == SIGQUIT) {
        g_run_flag = false;
    }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("usage: ./test_body_detector <model file path> "
               "<config file path> <image file path>\n");
        return -1;
    }
    char *model_path = argv[1];
    char *config_path = argv[2];
    char *image_path = argv[3];
    g_image_path = image_path;

    signal(SIGQUIT, sig_hanlder);

    int ret = 0;
    IXeyeComputation *xeye = NULL;
    do {
        create_xeye_computation_instance(model_path, config_path,
                                         ssd_computation_output_handler, \
                                         watchdog_handler, &xeye);
        if (0 != ret) {
            printf("failed to create xeye computation instance. ret:%d\n", ret);
            ret = -2;
        }
        int retry_count = 1;
        for (int i = 1; i <= retry_count; ++i) {
            ret = xeye->start();
            if (0 != ret) {
                printf("failed to start xeye computation instance. ret:%d\n", \
                       ret);
                if (retry_count == i) {
                    ret = -3;
                    break;
                } else {
                    sleep(1);
                }
            }
        }
    } while (false);

    if (0 != ret) {
        printf("xeye_computation init or start failed. destroy...\n");
        destroy_xeye_computation_instance(xeye);
        xeye = NULL;
        return ret;
    }

    g_run_flag = true;
    while (g_run_flag) {
        uint8_t *image_buf = (uint8_t *)malloc(8 * 1024 * 1024);
        if (NULL == image_buf) {
            printf("failed to malloc image_buf.\n");
            sleep(1);
            continue;
        }

        cv::Mat mat = cv::imread(image_path);
        if (NULL == mat.data) {
            printf("imread %s failed.\n", image_path);
            sleep(1);
            free(image_buf);
            continue;
        }
        XeyeInputContext input_context;
        input_context.timestamp = XeyeUtil::unix_timestamp();
        input_context.image_height = mat.rows;
        input_context.image_width = mat.cols;
        CameraInfo *camera_info = (CameraInfo *)input_context.extra;
        snprintf(camera_info->device_id, 16, "%s", "device-0");
        snprintf(camera_info->camera_id, 16, "%s", "camera-0");
        memcpy(image_buf, mat.datastart, mat.rows * mat.cols * 3);


        XeyeComputation *xeye_computation = (XeyeComputation *)xeye;
        XeyeConfig xeye_config = *(xeye_computation->xeye()->_xeye_config);
        xeye_config.enable_preprocess = 1;
        
        XeyeDataItem data_item;
        XeyeImage &xeye_image = data_item.xeye_image;
        xeye_image.org_image_type = IMAGETYPE_BGRBGR;
        xeye_image.org_image_data_type = IMAGEDATATYPE_UINT8;
        xeye_image.image_type = IMAGETYPE_RGBRGB;
        xeye_image.image_data_type = IMAGEDATATYPE_FP16;
        xeye_image.timestamp = input_context.timestamp;
        memcpy(xeye_image.extra, input_context.extra, EXTRA_IMAGE_INFO_SIZE);
        xeye_image.org_height = input_context.image_height;
        xeye_image.org_width = input_context.image_width;
        xeye_image.org_image = image_buf;
        ret = DlProcess::preprocess(xeye_config, data_item);
        if (0 != ret) {
            printf("preprocess failed. ret:%d\n", ret);
            free(image_buf);
            break;
        }

        input_context.timestamp = XeyeUtil::unix_timestamp();
        input_context.image_height = xeye_config.resize_height;
        input_context.image_width = xeye_config.resize_width;
        assert(xeye_image.height == xeye_config.resize_height);
        assert(xeye_image.width == xeye_config.resize_width);
        float *float_image_buf = (float *)image_buf;
        int count = xeye_image.height * xeye_image.width * 3; 
        for (int i = 0; i < count; ++i) {
            float_image_buf[i] = f16_to_f32(xeye_image.image[i]);
        }
        free(xeye_image.image);
        xeye_image.image = NULL;

        xeye->push_image(&input_context, float_image_buf, count * sizeof(float));
    }
    printf("after infinite loop.\n");

    xeye->stop();
    printf("after xeye::stop.\n");
    destroy_xeye_computation_instance(xeye);
    xeye = NULL;
    printf("after destroy_xeye_computation_instance.\n");

    printf("main thread exit.\n");
    return 0;
}

