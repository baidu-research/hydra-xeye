/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "body_detector_handler.h"
#include "image_handler.h"
#include "fp16_convert.h"
#include "nms.h"
#include "debug_def.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include <opencv2/opencv.hpp>

#include <vector>
#include <string>

std::string g_image_path;

void show_result_on_image(std::vector<SSDDetectedItem> &valid_objects) {
    static bool opencv_window_inited = false;
    if (!opencv_window_inited) {
        cv::namedWindow("face", CV_WINDOW_AUTOSIZE);
        cv::moveWindow("face", 1, 1);
        opencv_window_inited = true;
    }

    cv::Mat mat = cv::imread(g_image_path);

    float scale = 1.0;
#if 0
    cv::Size dsize = cv::Size(mat.cols * scale,
                              mat.rows * scale);
    cv::Mat display_mat = cv::Mat(dsize, CV_8UC3);
    cv::resize(mat, display_mat, dsize);
#else
    cv::Mat &display_mat = mat;
#endif

    std::vector<SSDDetectedItem>::iterator i;
    for (i = valid_objects.begin(); i != valid_objects.end(); ++i) {
        CvPoint lt;
        lt.x = (int)(i->x1 * (float)display_mat.cols);
        lt.y = (int)(i->y1 * (float)display_mat.rows);

        CvPoint rb;
        rb.x = (int)(i->x2 * (float)display_mat.cols);
        rb.y = (int)(i->y2 * (float)display_mat.rows);

        cv::rectangle(display_mat, lt, rb, CV_RGB(0, 255, 0));
        DEBUG_PRINT("show rect(%d,%d,%d,%d)\n", lt.x, lt.y, rb.x, rb.y);
    }

    cv::imshow("face", display_mat);
    cv::waitKey(1);
    //cv::imwrite("ssd_output_result.jpg", mat);
}

void body_detector_handler(XeyeDataItem *data_item) {
    if (NULL == data_item) {
        printf("error: data_item is NULL.\n");
        return;
    }
    DEBUG_PRINT("body_detector_handler.\n");

    XeyeMessage &xeye_message = *(data_item->xeye_message);
    XeyeImage &xeye_image = data_item->xeye_image;
    XeyeResult &xeye_result = *(data_item->xeye_result);
    DEBUG_PRINT("timestamp: %d result_data_size:%d image_count:%d ",
                "result embedded:%d\n"
                (int)xeye_image.timestamp, (int)xeye_result.result_size,
                (int)xeye_message.image_count, (int)xeye_result.embedded);

#if 1
    static struct timeval tv1;
    struct timeval tv2;
    gettimeofday(&tv2, NULL);
    int duration1 = (int)(tv2.tv_sec - tv1.tv_sec) * 1000 +
            (tv2.tv_usec - tv1.tv_usec) / 1000;
    printf("^^^^ interval %d ms.\n", duration1);
    tv1 = tv2;
#endif

    //filter result
    assert(0 == xeye_result.result_size % sizeof(SSDDetectedItem));
    SSDDetectedItem *item = NULL;
    if (0 == xeye_result.embedded) {
        assert(xeye_result.result_size > RESULT_DATA_SIZE);
        item = (SSDDetectedItem *)xeye_result.big_result_data;
    } else {
        assert(xeye_result.result_size <= RESULT_DATA_SIZE);
        item = (SSDDetectedItem *)xeye_result.result_data;
    }
    int count = (int)item->count;
    std::vector<SSDDetectedItem> valid_objects;
    valid_objects.reserve(count);

    std::vector<cv::Rect2f> body_rects;
    std::vector<float> body_scores;
    std::vector<cv::Rect2f> head_rects;
    std::vector<float> head_scores;

    std::vector<cv::Rect2f> valid_body_rects;
    std::vector<float> valid_body_scores;
    std::vector<cv::Rect2f> valid_head_rects;
    std::vector<float> valid_head_scores;

    float score_threashold = 0.3;
    for (int i = 0; i < count; ++i) {
        ++item;
        if (!(item->type < BODYPARTTYPE_MAX)) {
            printf("warning: invalid bodyparttype: %d\n",
                   (int)item->type);
            continue;
        }
        if (BODYPARTTYPE_BACKGROUND == item->type) {
            printf("warning: ignore background bodyparttype: %d\n",
                   (int)item->type);
            continue;
        }

        if (item->score < score_threashold || item->score > 1.0) {
            //DEBUG_PRINT("invalid score: %f\n", item->score);
            continue;
        }

        item->x1 = item->x1 < 0.0 ? 0.0 : item->x1;
        item->y1 = item->y1 < 0.0 ? 0.0 : item->y1;
        item->x2 = item->x2 < 0.0 ? 0.0 : item->x2;
        item->y2 = item->y2 < 0.0 ? 0.0 : item->y2;

        item->x1 = item->x1 > 1.0 ? 1.0 : item->x1;
        item->y1 = item->y1 > 1.0 ? 1.0 : item->y1;
        item->x2 = item->x2 > 1.0 ? 1.0 : item->x2;
        item->y2 = item->y2 > 1.0 ? 1.0 : item->y2;

        if (item->x1 >= item->x2 || item->y1 >= item->y2) {
            continue;
        }

        switch ((int)item->type) {
        case BODYPARTTYPE_BACKGROUND:
            printf("warning: met one background part.\n");
            break;
        case BODYPARTTYPE_BODY:
        {
            cv::Rect2f rect(item->x1, item->y1, item->x2 - item->x1, \
                            item->y2 - item->y1);
            body_rects.push_back(rect);
            body_scores.push_back(item->score);
            break;
        }
        case BODYPARTTYPE_HEAD:
        {
            cv::Rect2f rect(item->x1, item->y1, item->x2 - item->x1, \
                            item->y2 - item->y1);
            head_rects.push_back(rect);
            head_scores.push_back(item->score);
            break;
        }
        default:
            printf("warning: met one unknown part %d.\n", (int)item->type);
            break;
        }
    }

    //body nms
    nms2(body_rects, body_scores, valid_body_rects, valid_body_scores, \
         0.45);
    //head nms
    nms2(head_rects, head_scores, valid_head_rects, valid_head_scores, \
         0.45);

    //generate body info output
    assert(valid_body_rects.size() == valid_body_scores.size());
    for (int i = 0; i < valid_body_rects.size(); ++i) {
        SSDDetectedItem info;
        info.count = 0;
        info.type = BODYPARTTYPE_BODY;
        info.score = valid_body_scores[i];
        info.x1 = valid_body_rects[i].x;
        info.y1 = valid_body_rects[i].y;
        info.x2 = valid_body_rects[i].x + valid_body_rects[i].width;
        info.y2 = valid_body_rects[i].y + valid_body_rects[i].height;
        valid_objects.push_back(info);
        DEBUG_PRINT("valid_object type:%d score:%f x1:%f y1:%f x2:%f y2:%f\n",
               (int)info.type, info.score, info.x1, info.y1,
               info.x2, info.y2);
    }

    //generate head info output
    assert(valid_head_rects.size() == valid_head_scores.size());
    for (int i = 0; i < valid_head_rects.size(); ++i) {
        SSDDetectedItem info;
        info.count = 0;
        info.type = BODYPARTTYPE_HEAD;
        info.score = valid_head_scores[i];
        info.x1 = valid_head_rects[i].x;
        info.y1 = valid_head_rects[i].y;
        info.x2 = valid_head_rects[i].x + valid_head_rects[i].width;
        info.y2 = valid_head_rects[i].y + valid_head_rects[i].height;
        valid_objects.push_back(info);
        DEBUG_PRINT("valid_object type:%d score:%f x1:%f y1:%f x2:%f y2:%f\n",
               (int)info.type, info.score, info.x1, info.y1,
               info.x2, info.y2);
    }
    DEBUG_PRINT("valid objects: %d, total objects: %d\n",
           (int)valid_objects.size(), count);

    //show image
    show_result_on_image(valid_objects);
}

