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

#include <stdio.h>

int main(int argc, char **argv) {
    if (2 != argc) {
        printf("usage: ./test_cjson <json file path>\n");
        return -1;
    }
    char *json_file_path = argv[1];

    FileMap file_map;
    int ret = file_map.init(json_file_path);
    if (0 != ret) {
        printf("failed to create file mapping for file %s\n", json_file_path);
        return -2;
    }

    XeyeConfig process_config;
    ret = parse_config_file(&file_map, &process_config);
    if (0 != ret) {
        printf("failed to parse json file. ret:%d\n", ret);
        return -3;
    }

    printf("enable_preprocess: %d\n", process_config.enable_preprocess);
    printf("enable_resize: %d\n", process_config.enable_resize);
    printf("resize_height: %d\n", (int)process_config.resize_height);
    printf("resize_width: %d\n", (int)process_config.resize_width);
    printf("enable_mean: %d\n", process_config.enable_mean);
    printf("mean_b: %f\n", process_config.mean_b);
    printf("mean_g: %f\n", process_config.mean_g);
    printf("mean_r: %f\n", process_config.mean_r);
    printf("enable_scale: %d\n", process_config.enable_scale);
    printf("scale: %f\n", process_config.scale);
    return 0;
}
