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
#include "style_transfer_handler.h"
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
        printf("usage: ./test_style_transfer <model file path> "
               "<config file path> <image file path>\n");
        return -1;
    }
    char *model_path = argv[1];
    char *config_path = argv[2];
    char *image_path = argv[3];

    signal(SIGQUIT, sig_hanlder);

    int ret = 0;
    IXeyeComputation *xeye = NULL;
    do {
        ret = create_xeye_computation_instance(\
                    model_path, config_path, style_transfer_handler, \
                    watchdog_handler, &xeye);
        if (0 != ret) {
            printf("failed to create xeye computation instance. "\
                   "ret:%d\n", ret);
            ret = -2;
            break;
        }

        ret = xeye->start();
        if (0 != ret) {
            printf("failed to start xeye computation instance. ret:%d\n", \
                   ret);
            break;
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

        xeye->push_image(&input_context, &mat);
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
