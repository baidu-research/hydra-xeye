/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_SSD_OUTPUT_HANDLER_H
#define BAIDU_XEYE_SSD_OUTPUT_HANDLER_H

#include "xeye_data_type.h"

#include <string>

#pragma pack(push, 4)
struct SSDDetectedItem {
    float count;
    float type;
    float score;
    float x1;
    float y1;
    float x2;
    float y2;
};
#pragma pack(pop)

extern std::string g_image_path;

void ssd_output_handler(XeyeDataItem *data_item);

void ssd_computation_output_handler(XeyeDataItem *data_item);

#endif // BAIDU_XEYE_SSD_OUTPUT_HANDLER_H
