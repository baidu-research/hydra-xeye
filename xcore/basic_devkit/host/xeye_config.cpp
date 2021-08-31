/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "xeye_config.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_config_file(FileMap *config_file_map,
                      XeyeConfig *process_config) {
    if (NULL == config_file_map || NULL == process_config) {
        printf("faied to parse_config_file, invalid parameters.\n");
        return -1;
    }

    int ret = 0;
    char *json_buf = NULL;
    cJSON *config = NULL;

    do {
        json_buf = (char *)malloc(config_file_map->size() + 1);
        if (NULL == json_buf) {
            printf("failed to malloc json_buf size:%d\n",
                   (int)config_file_map->size() + 1);
            ret = -2;
            break;
        }
        memcpy(json_buf, config_file_map->map(), config_file_map->size());
        json_buf[config_file_map->size()] = 0;

        config = cJSON_Parse(json_buf);
        if (NULL == config) {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (NULL != error_ptr) {
                printf("parse failed, error before: %s\n", error_ptr);
            }
            ret = -3;
            break;
        }

        const cJSON *enable_preprocess =
                cJSON_GetObjectItemCaseSensitive(config, "enable_preprocess");
        if (NULL == enable_preprocess) {
            printf("enable_preprocess not found!\n");
            ret = -4;
            break;
        }
        if (cJSON_IsNumber(enable_preprocess)) {
            process_config->enable_preprocess = enable_preprocess->valueint;
        } else {
            printf("enable_preprocess is not number.\n");
            ret = -5;
            break;
        }

        const cJSON *preprocess =
                cJSON_GetObjectItemCaseSensitive(config, "preprocess");
        if (NULL == preprocess) {
            printf("preprocess not found!\n");
            ret = -6;
            break;
        }

        cJSON *enable_resize =
                cJSON_GetObjectItemCaseSensitive(preprocess, "enable_resize");
        if (NULL == enable_resize) {
            printf("enable_resize not found!\n");
            ret = -7;
            break;
        }
        if (cJSON_IsNumber(enable_resize)) {
            process_config->enable_resize = enable_resize->valueint;
        } else {
            printf("enable_resize is not number.\n");
            ret = -8;
            break;
        }

        cJSON *resize =
                cJSON_GetObjectItemCaseSensitive(preprocess, "resize");
        if (NULL == resize) {
            printf("resize not found!\n");
            ret = -9;
            break;
        }

        cJSON *height =
                cJSON_GetObjectItemCaseSensitive(resize, "height");
        if (NULL == height) {
            printf("height not found!\n");
            ret = -10;
            break;
        }
        if (cJSON_IsNumber(height)) {
            process_config->resize_height = height->valueint;
        } else {
            printf("height is not number.\n");
            ret = -11;
            break;
        }

        cJSON *width =
                cJSON_GetObjectItemCaseSensitive(resize, "width");
        if (NULL == width) {
            printf("width not found!\n");
            ret = -12;
            break;
        }
        if (cJSON_IsNumber(width)) {
            process_config->resize_width = width->valueint;
        } else {
            printf("width is not number.\n");
            ret = -13;
            break;
        }

        cJSON *enable_mean =
                cJSON_GetObjectItemCaseSensitive(preprocess, "enable_mean");
        if (NULL == enable_mean) {
            printf("enable_mean not found!\n");
            ret = -14;
            break;
        }
        if (cJSON_IsNumber(enable_mean)) {
            process_config->enable_mean = enable_mean->valueint;
        } else {
            printf("enable_mean is not number.\n");
            ret = -15;
            break;
        }

        cJSON *mean =
                cJSON_GetObjectItemCaseSensitive(preprocess, "mean");
        if (NULL == mean) {
            printf("mean not found!\n");
            ret = -16;
            break;
        }

        cJSON *b =
                cJSON_GetObjectItemCaseSensitive(mean, "b");
        if (NULL == b) {
            printf("b not found!\n");
            ret = -17;
            break;
        }
        if (cJSON_IsNumber(b)) {
            process_config->mean_b = (float)b->valuedouble;
        } else {
            printf("b is not number.\n");
            ret = -18;
            break;
        }

        cJSON *g =
                cJSON_GetObjectItemCaseSensitive(mean, "g");
        if (NULL == g) {
            printf("g not found!\n");
            ret = -19;
            break;
        }
        if (cJSON_IsNumber(g)) {
            process_config->mean_g = (float)g->valuedouble;
        } else {
            printf("g is not number.\n");
            ret = -20;
            break;
        }

        cJSON *r =
                cJSON_GetObjectItemCaseSensitive(mean, "r");
        if (NULL == r) {
            printf("r not found!\n");
            ret = -21;
            break;
        }
        if (cJSON_IsNumber(r)) {
            process_config->mean_r = (float)r->valuedouble;
        } else {
            printf("r is not number.\n");
            ret = -22;
            break;
        }

        cJSON *enable_scale =
                cJSON_GetObjectItemCaseSensitive(preprocess,
                                                 "enable_scale");
        if (NULL == enable_scale) {
            printf("enable_scale not found!\n");
            ret = -23;
            break;
        }
        if (cJSON_IsNumber(enable_scale)) {
            process_config->enable_scale = enable_scale->valueint;
        } else {
            printf("enable_scale is not number.\n");
            ret = -24;
            break;
        }

        cJSON *scale =
                cJSON_GetObjectItemCaseSensitive(preprocess,
                                                 "scale");
        if (NULL == scale) {
            printf("scale not found!\n");
            ret = -25;
            break;
        }
        if (cJSON_IsNumber(scale)) {
            process_config->scale = (float)scale->valuedouble;
        } else {
            printf("scale is not number.\n");
            ret = -26;
            break;
        }

        const cJSON *result =
                cJSON_GetObjectItemCaseSensitive(config, "result");
        if (NULL == result) {
            printf("result not found!\n");
            ret = -27;
            break;
        }

        cJSON *with_original_image =
                cJSON_GetObjectItemCaseSensitive(result, "with_original_image");
        if (NULL == with_original_image) {
            printf("with_original_image not found!\n");
            ret = -28;
            break;
        }
        if (cJSON_IsNumber(with_original_image)) {
            process_config->with_original_image = with_original_image->valueint;
        } else {
            printf("with_original_image is not number.\n");
            ret = -29;
            break;
        }
    } while (false);

    free(json_buf);
    json_buf = NULL;
    cJSON_Delete(config);
    config = NULL;
    return ret;
}
