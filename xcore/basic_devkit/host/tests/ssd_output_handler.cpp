/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "ssd_output_handler.h"
#include "image_handler.h"
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

#define SUNNY 1
#define SCORE_THRESHOLD 0.16
#define MAX_BOX_NUM 10
int img_cnt = 0;

std::string g_image_path;

void ssd_output_handler(XeyeDataItem *data_item) {
    if (NULL == data_item) {
        printf("error: data_item is NULL.\n");
        return;
    }
    DEBUG_PRINT("ssd_output_handler.\n");

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
#if !SUNNY
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

    float score_threashold = 0.6;
    for (int i = 0; i < count; ++i) {
        ++item;
//        printf("[%d] type:%d score:%f x1:%f y1:%f x2:%f y2:%f\n",
//               i, (int)item->type, item->score, item->x1, item->y1,
//               item->x2, item->y2);
        if (item->score < score_threashold || item->score > 1.0) {
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

        valid_objects.push_back(*item);
        DEBUG_PRINT("valid_object [%d] score:%f x1:%f y1:%f x2:%f y2:%f\n",
               i, item->score, item->x1, item->y1,
               item->x2, item->y2);
    }
    DEBUG_PRINT("valid objects: %d, total objects: %d\n",
           (int)valid_objects.size(), count);

    //print image
    assert(0 != xeye_message.image_count);
    uint8_t *bgr_image = NULL;
    int ret = separate_rgb_to_combine_bgr(xeye_image.org_height, \
                                         xeye_image.org_width, \
                                         xeye_image.org_image, &bgr_image);
    if (0 != ret) {
        printf("separate_rgb_to_combine_bgr failed. ret:%d\n", ret);
        return;
    }

    static bool opencv_window_inited = false;
    if (!opencv_window_inited) {
        cv::namedWindow("face", CV_WINDOW_AUTOSIZE);
        cv::moveWindow("face", 1, 1);
        opencv_window_inited = true;
    }
    DEBUG_PRINT("org_height:%d org_width:%d\n", xeye_image.org_height, \
                xeye_image.org_width);
    cv::Mat mat = cv::Mat(xeye_image.org_height, xeye_image.org_width, \
                          CV_8UC3, bgr_image);

    float scale = 1.0;
#if 0
    cv::Size dsize = cv::Size(xeye_image.org_width * scale,
                              xeye_image.org_height * scale);
    cv::Mat display_mat = cv::Mat(dsize, CV_8UC3);
    cv::resize(mat, display_mat, dsize);
#else
    cv::Mat &display_mat = mat;
#endif

    std::vector<SSDDetectedItem>::iterator i;
    for (i = valid_objects.begin(); i != valid_objects.end(); ++i) {
        CvPoint lt;
        lt.x = (int)(i->x1 * (float)xeye_image.org_width * scale);
        lt.y = (int)(i->y1 * (float)xeye_image.org_height * scale);

        CvPoint rb;
        rb.x = (int)(i->x2 * (float)xeye_image.org_width * scale);
        rb.y = (int)(i->y2 * (float)xeye_image.org_height * scale);

        cv::rectangle(display_mat, lt, rb, CV_RGB(0, 255, 0));
        DEBUG_PRINT("show rect(%d,%d,%d,%d)\n", lt.x, lt.y, rb.x, rb.y);
    }

    cv::imshow("face", display_mat);
    cv::waitKey(1);
    //cv::imwrite("ssd_output_result.jpg", mat);

    free(bgr_image);
    bgr_image = NULL;
#else

    img_cnt++;
    printf("==== No. %6d\n",img_cnt);



    cv::Mat incoming_img;
    cv::Mat outgoing_img;


    if(0!=xeye_message.image_count)
    {
    	incoming_img = cv::Mat(cv::Size(1280, 720*1.5), CV_8UC1, xeye_image.org_image, 0);
    	outgoing_img = cv::Mat(720, 1280, CV_8UC3);
    	cv::cvtColor(incoming_img, outgoing_img, CV_YUV2RGB_YV12, 3);
    }

    SSDDetectedItem *item = NULL;
    if (0 == xeye_result.embedded) {
        item = (SSDDetectedItem *)xeye_result.big_result_data;
    } else {
        item = (SSDDetectedItem *)xeye_result.result_data;
    }
    int count = (int)item->count;
    int ValidCnt = 0;
    std::vector<SSDDetectedItem> valid_objects;
    valid_objects.reserve(count);
    
    float score_threashold = SCORE_THRESHOLD;
    for (int i = 0; i < count; ++i) {
        ++item;
//        printf("[%d] type:%d score:%f x1:%f y1:%f x2:%f y2:%f\n",
//               i, (int)item->type, item->score, item->x1, item->y1,
//               item->x2, item->y2);
        if (item->score < score_threashold || item->score > 1.0) {
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

        valid_objects.push_back(*item);

        DEBUG_PRINT("valid_object [%d] score:%f x1:%f y1:%f x2:%f y2:%f\n",
               i, item->score, item->x1, item->y1,
               item->x2, item->y2);        
        ValidCnt++;
        if(ValidCnt >= MAX_BOX_NUM) break;
    }
    DEBUG_PRINT("valid objects: %d, total objects: %d\n",
           (int)valid_objects.size(), count);

    std::vector<SSDDetectedItem>::iterator i;
    for (i = valid_objects.begin(); i != valid_objects.end(); ++i) {
        CvPoint lt;
        lt.x = (int)(i->x1 * (float)1280);
        lt.y = (int)(i->y1 * (float)720);

        CvPoint rb;
        rb.x = (int)(i->x2 * (float)1280);
        rb.y = (int)(i->y2 * (float)720);
        if(0!=xeye_message.image_count){

        cv::rectangle(outgoing_img, lt, rb, CV_RGB(0, 255, 0));
        }
        DEBUG_PRINT("show rect(%d,%d,%d,%d)\n", lt.x, lt.y, rb.x, rb.y);
    }


    // FILE *fp = NULL;
    // fp =  fopen("nnresult_pc.raw", "wb"); 
    // if(fp==NULL)
    // {    printf("==== -1 ====\n"); }
    // fwrite(xeye_message.result_info.result_data, 21504, 1, fp);
    // fclose(fp);
    // fp = NULL;

    // FILE *fp = NULL;
    // fp =  fopen("img.yuv", "wb"); 
    // if(fp==NULL)
    // {    printf("==== -1 ====\n"); }
    // fwrite(xeye_image.org_image, 1280*720*3/2, 1, fp);
    // fclose(fp);
    // fp = NULL;

    if(0!=xeye_message.image_count){

    cv::imshow("rgb", outgoing_img);
    cv::waitKey(1);
    }
#endif

}

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

void ssd_computation_output_handler(XeyeDataItem *data_item) {
    if (NULL == data_item) {
        printf("error: data_item is NULL.\n");
        return;
    }
    DEBUG_PRINT("ssd_output_handler.\n");

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



    img_cnt++;

    uint8_t *image_buf = (uint8_t *)malloc(8 * 1024 * 1024);
    if (NULL == image_buf) {
    	printf("failed to malloc image_buf.\n");
    }

    FILE *fp = NULL;
    fp =  fopen(g_image_path.c_str(), "rb"); 
    if(fp==NULL)
    {    
    	printf("image open fail\n"); 
    }
    fread((uint8_t *)image_buf, 720*1280*3/2, 1, fp);
    fclose(fp);
    fp = NULL;

    xeye_image.org_image = image_buf;
    cv::Mat incoming_img = cv::Mat(cv::Size(1280, 720*1.5), CV_8UC1, xeye_image.org_image, 0);
    cv::Mat outgoing_img = cv::Mat(720, 1280, CV_8UC3);
    cv::cvtColor(incoming_img, outgoing_img, CV_YUV2RGB_YV12, 3);

    printf("==== Img No. %6d\n",img_cnt);


    SSDDetectedItem *item = NULL;
    if (0 == xeye_result.embedded) {
        item = (SSDDetectedItem *)xeye_result.big_result_data;
    } else {
        item = (SSDDetectedItem *)xeye_result.result_data;
    }
    int count = (int)item->count;
    int ValidCnt = 0;
    std::vector<SSDDetectedItem> valid_objects;
    valid_objects.reserve(count);
    
    float score_threashold = SCORE_THRESHOLD;
    for (int i = 0; i < count; ++i) {
        ++item;
//        printf("[%d] type:%d score:%f x1:%f y1:%f x2:%f y2:%f\n",
//               i, (int)item->type, item->score, item->x1, item->y1,
//               item->x2, item->y2);
        if (item->score < score_threashold || item->score > 1.0) {
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

        valid_objects.push_back(*item);

        DEBUG_PRINT("valid_object [%d] score:%f x1:%f y1:%f x2:%f y2:%f\n",
               i, item->score, item->x1, item->y1,
               item->x2, item->y2);        
        ValidCnt++;
        if(ValidCnt >= MAX_BOX_NUM) break;
    }
    DEBUG_PRINT("valid objects: %d, total objects: %d\n",
           (int)valid_objects.size(), count);

    std::vector<SSDDetectedItem>::iterator i;
    for (i = valid_objects.begin(); i != valid_objects.end(); ++i) {
        CvPoint lt;
        lt.x = (int)(i->x1 * (float)1280);
        lt.y = (int)(i->y1 * (float)720);

        CvPoint rb;
        rb.x = (int)(i->x2 * (float)1280);
        rb.y = (int)(i->y2 * (float)720);

        cv::rectangle(outgoing_img, lt, rb, CV_RGB(0, 255, 0));
        DEBUG_PRINT("show rect(%d,%d,%d,%d)\n", lt.x, lt.y, rb.x, rb.y);
    }


    // FILE *fp = NULL;
    // fp =  fopen("nnresult_pc.raw", "wb"); 
    // if(fp==NULL)
    // {    printf("==== -1 ====\n"); }
    // fwrite(xeye_message.result_info.result_data, 21504, 1, fp);
    // fclose(fp);
    // fp = NULL;

    // FILE *fp = NULL;
    // fp =  fopen("img.yuv", "wb"); 
    // if(fp==NULL)
    // {    printf("==== -1 ====\n"); }
    // fwrite(xeye_image.org_image, 1280*720*3/2, 1, fp);
    // fclose(fp);
    // fp = NULL;

    cv::imshow("rgb", outgoing_img);
    cv::waitKey(1);
}
