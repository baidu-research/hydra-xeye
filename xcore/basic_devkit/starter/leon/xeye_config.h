/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_XEYE_CONFIG_H
#define BAIDU_XEYE_XEYE_CONFIG_H

#include "xeye_data_type.h"

#include <stdint.h>
#include <stdio.h>

struct FileMap {
    void *map;
    uint32_t size;

    FileMap() {
        map = NULL;
        size = 0;
    }
};

int parse_config_file(FileMap *config_file_map, \
                      XeyeConfig *xeye_config);

#endif // BAIDU_XEYE_XEYE_CONFIG_H
