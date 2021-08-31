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

#include "file_map.h"
#include "xeye_data_type.h"

int parse_config_file(FileMap *config_file_map, \
                      XeyeConfig *process_config);

#endif // BAIDU_XEYE_XEYE_CONFIG_H
