/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "dl_process.h"
#include "fp16_convert.h"
#include "debug_def.h"

#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <assert.h>

int DlProcess::preprocess(XeyeConfig &config, XeyeDataItem &data_item) {
    if (0 != config.enable_preprocess) {
        XeyeImage &xeye_image = data_item.xeye_image;
        if (NULL == xeye_image.org_image) {
            printf("error: has preprocess, org_image is NULL.\n");
            return -1;
        }
        if (xeye_image.org_image_type != IMAGETYPE_BGRBGR) {
            printf("error: has preprocess, org_image_type must be "
                   "IMAGETYPE_BGRBGR.\n");
            return -2;
        }
        if (xeye_image.image_type != IMAGETYPE_RGBRGB) {
            printf("error: has preprocess, image_type must be "
                   "IMAGETYPE_RGBRGB.\n");
            return -3;
        }
        if (xeye_image.org_image_data_type != IMAGEDATATYPE_UINT8) {
            printf("error: has preprocess, org_image_data_type must be "
                   "IMAGEDATATYPE_UINT8.\n");
            return -4;
        }
        if (xeye_image.image_data_type != IMAGEDATATYPE_FP16) {
            printf("error: has preprocess, image_data_type must be "
                   "IMAGEDATATYPE_FP16.\n");
            return -5;
        }

        uint8_t* org_image = xeye_image.org_image;
        assert(NULL != org_image);
        float* image_buf = NULL;
        uint32_t real_height = 0;
        uint32_t real_width = 0;
        int ret = 0;

        do {
            if (0 != config.enable_resize) {
                real_height = config.resize_height;
                real_width = config.resize_width;
                image_buf = (float*)malloc(real_height * real_width * \
                                           3 * sizeof(float));
                if (NULL == image_buf) {
                    printf("failed to malloc image_buf size: %d\n",
                           (int)(real_height * real_width * 3 * \
                                 sizeof(float)));
                    ret = -6;
                    break;
                }

                cv::Size dsize = cv::Size(real_width, real_height);
                cv::Mat resized_mat(real_height, real_width, CV_8UC3);
                if (xeye_image.org_mat.data != NULL) {
                    cv::resize(xeye_image.org_mat, resized_mat, dsize);
                } else {
                    cv::Mat input_mat(xeye_image.org_height, \
                                      xeye_image.org_width, CV_8UC3, org_image);
                    cv::resize(input_mat, resized_mat, dsize);
                }

                for (int i = 0; i < (int)real_height; ++i) {
                    for (int j = 0; j < (int)real_width; ++j) {
                        uint8_t *bgr_pixel = resized_mat.ptr(i, j);
                        int pos = (i * real_width + j) * 3;
                        for (int k = 0; k < 3; ++k) {
                            image_buf[pos + k] = bgr_pixel[k];
                        }
                    }
                }
            } else {
                real_height = xeye_image.org_height;
                real_width = xeye_image.org_width;
                image_buf = (float*)malloc(real_height * real_width * \
                                           3 * sizeof(float));
                if (NULL == image_buf) {
                    printf("failed to malloc image_buf size: %d\n",
                           (int)(real_height * real_width * 3 * \
                                 sizeof(float)));
                    ret = -7;
                    break;
                }
                for (int i = 0; i < (int)real_height; ++i) {
                    for (int j = 0; j < (int)real_width; ++j) {
                        int pos = (i * real_width + j) * 3;
                        for (int k = 0; k < 3; ++k) {
                            image_buf[pos + k] = org_image[pos + k];
                        }
                    }
                }
            }

            if (0 != config.enable_mean) {
                uint8_t bgr_mean[3] = {0, 0, 0};
                bgr_mean[0] = (uint8_t)config.mean_b;
                bgr_mean[1] = (uint8_t)config.mean_g;
                bgr_mean[2] = (uint8_t)config.mean_r;
                for (int i = 0; i < (int)real_height; ++i) {
                    for (int j = 0; j < (int)real_width; ++j) {
                        int pos = (i * real_width + j) * 3;
                        for (int k = 0; k < 3; ++k) {
                            image_buf[pos + k] -= (float)bgr_mean[k];
                        }
                    }
                }
            }

            if (0 != config.enable_scale) {
                float f = config.scale;
                for (int i = 0; i < (int)real_height; ++i) {
                    for (int j = 0; j < (int)real_width; ++j) {
                        int pos = (i * real_width + j) * 3;
                        for (int k = 0; k < 3; ++k) {
                            image_buf[pos + k] *= f;
                        }
                    }
                }
            }

            xeye_image.height = real_height;
            xeye_image.width = real_width;

            int count = real_height * real_width * 3;
            xeye_image.image = (uint16_t *)malloc(count * sizeof(uint16_t));
            if (NULL == xeye_image.image) {
                printf("failed to malloc %d for image.\n",
                       (int)(count * sizeof(uint16_t)));
                ret = -8;
                break;
            }
            for (int i = 0; i < count; ++i) {
                xeye_image.image[i] = f32_to_f16(image_buf[i]);
            }
            ret = 0;
        } while (false);

        free(image_buf);
        image_buf = NULL;
        //do not forget to free xeye_image.image in future.
        return ret;
    } else {
        XeyeImage &xeye_image = data_item.xeye_image;
        if (NULL != xeye_image.org_image) {
            printf("warning: no preprocess, "\
                   "xeye_image.org_image should be NULL.\n");
        }
        if (NULL == xeye_image.float_image) {
            printf("error: no preprocess, xeye_image.float_image is NULL.\n");
            return -1;
        }
        if (xeye_image.image_type != IMAGETYPE_RGBRGB) {
            printf("error: no preprocess, image_type must be "
                   "IMAGETYPE_RGBRGB.\n");
            return -2;
        }
        if (xeye_image.image_data_type != IMAGEDATATYPE_FP16) {
            printf("error: no preprocess, image_data_type must be "
                   "IMAGEDATATYPE_FP16.\n");
            return -3;
        }

        int count = xeye_image.height * xeye_image.width * 3;
        xeye_image.image = (uint16_t *)malloc(count * sizeof(uint16_t));
        if (NULL == xeye_image.image) {
            printf("failed to malloc %d for image.\n",
                   (int)(count * sizeof(uint16_t)));
            return -4;
        }
        for (int i = 0; i < count; ++i) {
            xeye_image.image[i] = f32_to_f16(xeye_image.float_image[i]);
        }
        return 0;
    }
}

int DlProcess::postprocess(XeyeConfig &config, XeyeDataItem &data_item) {
    UNUSED(config);
    if (NULL == data_item.xeye_message) {
        printf("data_item.xeye_message is NULL.\n");
        return -1;
    }
    XeyeMessage &xeye_message = *(data_item.xeye_message);
    if (1 != xeye_message.with_result) {
        printf("xeye_message without result.\n");
        return -2;
    }

    data_item.xeye_result = new XeyeResult;
    int count = xeye_message.result_info.result_size / sizeof(uint16_t);
    data_item.xeye_result->result_size = count * sizeof(float);
    data_item.xeye_result->embedded = xeye_message.result_info.embedded;

    int ret = 0;
    if (1 == xeye_message.result_info.embedded) {
        assert(xeye_message.result_info.result_size <= RESULT_DATA_SIZE);
        float *result = (float *)data_item.xeye_result->result_data;
        uint16_t *org_result = (uint16_t *)xeye_message.result_info.result_data;
        for (int i = 0; i < count; ++i) {
            result[i] = f16_to_f32(org_result[i]);
        }
    } else {
        assert(xeye_message.result_info.result_size > RESULT_DATA_SIZE);
        data_item.xeye_result->big_result_data = \
            (uint8_t *)malloc(data_item.xeye_result->result_size);
        if (NULL != data_item.xeye_result->big_result_data) {
            float *result = (float *)data_item.xeye_result->big_result_data;
            uint16_t *org_result = \
                (uint16_t *)xeye_message.result_info.big_result_data;
            for (int i = 0; i < count; ++i) {
                result[i] = f16_to_f32(org_result[i]);
            }
        } else {
            printf("failed to malloc result_size:%d\n", \
                   (int)data_item.xeye_result->result_size);
            ret = -3;
        }
    }
    return ret;
}
