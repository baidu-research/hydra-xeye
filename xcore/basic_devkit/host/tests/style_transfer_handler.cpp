/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "style_transfer_handler.h"
#include "debug_def.h"
#include "xeye_util.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include <opencv2/opencv.hpp>

#include <vector>
#include <string>

inline int clip(int value, int min, int max) {
    if (value < min) {
        value = min;
    } else if (value > max) {
        value = max;
    }
    return value;
}

void style_transfer_handler(XeyeDataItem *data_item) {
    if (NULL == data_item) {
        printf("error: data_item is NULL.\n");
        return;
    }
    DEBUG_PRINT("style_transfer_handler.\n");

    XeyeMessage &xeye_message = *(data_item->xeye_message);
    XeyeImage &xeye_image = data_item->xeye_image;
    XeyeResult &xeye_result = *(data_item->xeye_result);
    DEBUG_PRINT("timestamp: %lu result_data_size:%d image_count:%d ",
                "result embedded:%d\n"
                xeye_image.timestamp, (int)xeye_result.result_size,
                (int)xeye_message.image_count, (int)xeye_result.embedded);

#if 0
    uint64_t ut = XeyeUtil::unix_timestamp();
    uint64_t ut1 = data_item->xeye_message->image_info[0].timestamp;
    uint64_t ut2 = data_item->xeye_image.timestamp;
    assert(ut1 == ut2);
    //printf("image_timestamp:%lu now_timestamp:%lu\n", ut1, ut);
    printf("#### response time:%lu\n", ut - ut1);
#endif

#if 1
    static struct timeval tv1;
    struct timeval tv2;
    gettimeofday(&tv2, NULL);
    int duration1 = (int)(tv2.tv_sec - tv1.tv_sec) * 1000 +
            (tv2.tv_usec - tv1.tv_usec) / 1000;
    printf("^^^^ interval %d ms.\n", duration1);
    tv1 = tv2;
#endif

    static bool opencv_window_inited = false;
    if (!opencv_window_inited) {
        cv::namedWindow("style_transfer", CV_WINDOW_AUTOSIZE);
        cv::moveWindow("style_transfer", 1, 1);
        opencv_window_inited = true;
    }

    assert(0 == xeye_result.embedded);
    assert(NULL != xeye_result.big_result_data);
    int h = 259;
    int w = 259;
    assert(h * w * 3 * sizeof(float) == xeye_result.result_size);
    uint8_t *bgr_image = (uint8_t *)malloc(h * w * 3 * sizeof(uint8_t));
    assert(NULL != bgr_image);
    float *big_result_data = (float *)xeye_result.big_result_data;
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            float *float_pixel = big_result_data + i * j * 3;
            uint8_t *bgr_pixel = bgr_image + i * j * 3;
            for (int k = 0; k < 3; ++k) {
                bgr_pixel[k] = (uint8_t)clip(float_pixel[2 - k], 0, 255);
            }
        }
    }
    cv::Mat mat = cv::Mat(h, w, CV_8UC3, bgr_image);

    float scale = 1.0;
#if 0
    cv::Size dsize = cv::Size(w * scale, h * scale);
    cv::Mat display_mat = cv::Mat(dsize, CV_8UC3);
    cv::resize(mat, display_mat, dsize);
#else
    cv::Mat &display_mat = mat;
#endif

    cv::imshow("style_transfer", display_mat);
    cv::waitKey(1);
    //cv::imwrite("style_transfer.jpg", mat);

    free(bgr_image);
    bgr_image = NULL;
}
