/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "xeye_interface.h"
#include "ssd_output_handler.h"
#include "watchdog_handler.h"

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <opencv2/opencv.hpp>

#include <atomic>
#include <string>

std::atomic<bool> g_run_flag;

void sig_hanlder(int sig_type) {
    printf("enter sig_handler sig_type:%d\n", sig_type);
    if (sig_type == SIGQUIT || sig_type == SIGINT) {
        printf("set g_run_flag to flase.\n");
        g_run_flag = false;
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("usage: ./test_xeye_sdk <model file path> "
               "<config file path>\n");
        return -1;
    }
    char *model_path = argv[1];
    char *config_path = argv[2];

    signal(SIGQUIT, sig_hanlder);
    signal(SIGINT, sig_hanlder);

    int ret = 0;
    IXeye *xeye = NULL;
    do {
        ret = create_xeye_instance(model_path, config_path, \
                                   ssd_output_handler, watchdog_handler, \
                                   &xeye);
        if (0 != ret) {
            printf("failed to create xeye instance. ret:%d\n", ret);
            ret = -2;
            break;
        }

        int retry_count = 1;
        for (int i = 1; i <= retry_count; ++i) {
            ret = xeye->start();
            if (0 != ret) {
                printf("failed to start xeye instance. ret:%d\n", ret);
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
        printf("xeye init or start failed. destroy...\n");
        destroy_xeye_instance(xeye);
        xeye = NULL;
        return ret;
    }

    g_run_flag = true;
    while (g_run_flag) {
        printf("host heart beat!\n");
        sleep(5);
    }
    printf("after infinite loop.\n");

    xeye->stop();
    printf("after xeye::stop.\n");
    destroy_xeye_instance(xeye);
    xeye = NULL;
    printf("after destroy_xeye_instance.\n");

    printf("main thread exit.\n");
    return 0;
}

