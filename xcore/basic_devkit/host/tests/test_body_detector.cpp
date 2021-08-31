/*
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
#include "body_detector_handler.h"
#include "watchdog_handler.h"

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

    signal(SIGQUIT, sig_hanlder);//设置g_run_flag的值

    int ret = 0;
    IXeyeComputation *xeye = NULL;
    do {
        int retry_count = 1;
        for (int i = 1; i <= retry_count; ++i) {
            ret = create_xeye_computation_instance(model_path, config_path, \
                                             body_detector_handler, \
                                             watchdog_handler, &xeye);//判断参数是否非法
            if (0 != ret) {
                printf("failed to create xeye computation instance. "\
                       "ret:%d\n", ret);
                if (retry_count == i) {
                    ret = -2;
                    break;
                } else {
                    sleep(1);
                }
            } else {
                printf("create_xeye_computation_instance successfully.\n");
                break;
            }
        }
        if (0 != ret) {
            break;
        }

        ret = xeye->start();
        if (0 != ret) {
            printf("failed to start xeye computation instance. ret:%d\n", \
                   ret);
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
//        uint8_t *image_buf = (uint8_t *)malloc(8 * 1024 * 1024);
//        if (NULL == image_buf) {
//            printf("failed to malloc image_buf.\n");
//            sleep(1);
//            continue;
//        }

        cv::Mat mat = cv::imread(image_path);
        if (NULL == mat.data) {
            printf("imread %s failed.\n", image_path);
            sleep(1);
            continue;
        }
        XeyeInputContext input_context;
        input_context.timestamp = XeyeUtil::unix_timestamp();
        input_context.image_height = mat.rows;
        input_context.image_width = mat.cols;
        CameraInfo *camera_info = (CameraInfo *)input_context.extra;
        snprintf(camera_info->device_id, 16, "%s", "device-0");
        snprintf(camera_info->camera_id, 16, "%s", "camera-0");
//        memcpy(image_buf, mat.datastart, mat.rows * mat.cols * 3);

        xeye->push_image(&input_context, &mat);
//        xeye->push_image(&input_context, image_path);
//        xeye->push_image(&input_context, image_buf, \
//                         input_context.image_height * \
//                         input_context.image_width * 3);
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
