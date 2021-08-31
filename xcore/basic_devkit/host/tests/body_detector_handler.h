/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BODY_DETECTOR_HANDLER_H
#define BODY_DETECTOR_HANDLER_H

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

enum BodyPartType {
    BODYPARTTYPE_BACKGROUND = 0,
    BODYPARTTYPE_BODY = 1,
    BODYPARTTYPE_HEAD = 2,
    BODYPARTTYPE_MAX,
};

extern std::string g_image_path;

void body_detector_handler(XeyeDataItem *data_item);

#endif // BODY_DETECTOR_HANDLER_H
