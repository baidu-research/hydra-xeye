/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_XEYE_MESSAGE_DATA_TYPE_H
#define BAIDU_XEYE_XEYE_MESSAGE_DATA_TYPE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MESSAGE_DATA_ALIGN (4)

#define XEYE_MESSAGE_SIZE (64 * 1024)
#define CONFIG_FILE_SIZE (4 * 1024)
#define RESULT_DATA_SIZE (32 * 1024)
#define EXTRA_IMAGE_INFO_SIZE (128)
#define IMAGE_COUNT (32)
#define MODEL_COUNT (8)
#define USER_FIELDS_SIZE (8 * 1024)

#define XEYE_MAGIC (0x89ABCDEF)

#pragma pack(push, MESSAGE_DATA_ALIGN)

enum XeyeMode {
    XEYEMODE_UNKNOWN = 0,
    XEYEMODE_STANDARD, //xeye with camera
    XEYEMODE_COMPUTATION, //xeye without camera
    XEYEMODE_MAX,
};

struct ConfigInfo {
    uint32_t xeye_mode; //XeyeMode
    uint32_t config_file_size;
    uint8_t config_file_data[CONFIG_FILE_SIZE];
};

struct ModelInfo {
    uint32_t model_file_size;
    uint32_t reserve;
    uint8_t model_name[16];
};

enum ImageType {
    IMAGETYPE_UNKNOWN = 0,
    IMAGETYPE_YUV,
    IMAGETYPE_RGBRGB,
    IMAGETYPE_BGRBGR,
    IMAGETYPE_RRGGBB,
    IMAGETYPE_BBGGRR,
};

enum ImageDataType {
    IMAGEDATATYPE_UNKNOWN = 0,
    IMAGEDATATYPE_UINT8,
    IMAGEDATATYPE_FP16,
    IMAGEDATATYPE_FLOAT,
};

struct ImageInfo {
    uint64_t timestamp; //image's capture timestamp
    uint32_t image_type; //ImageType
    uint32_t image_data_type; //ImageDataType
    uint32_t size; //total bytes
    uint32_t height; //image height
    uint32_t width; //image width
    uint32_t reserve;
    uint8_t extra[EXTRA_IMAGE_INFO_SIZE]; //for example camera_id, device_id
};

struct ResultInfo {
    uint32_t embedded; //1 - internal result, 0 - external result in other package
    uint32_t result_size; //total result size
    uint8_t result_data[RESULT_DATA_SIZE]; //when embedded is 1, reuslt stores here
    uint64_t big_result_data; //when embedded is 0, big_result_data point to a big buffer
};

enum ConnectTarget {
    CONNECTTARGET_UNKNOWN = 0,
    CONNECTTARGET_HOST,
    CONNECTTARGET_XEYE,
};

enum MessageType {
    MESSAGETYPE_UNKNOWN = 0,
    MESSAGETYPE_CONFIG_XEYE, //from Host to Xeye
    MESSAGETYPE_STOP_XEYE, //from Host to Xeye
    MESSAGETYPE_SET_INPUT, //from Host to Xeye
    MESSAGETYPE_SET_OUTPUT, //from Xeye to Host
};

struct XeyeMessage {
    uint32_t magic; //0xABCD
    uint32_t sender; //ConnectTarget, sender mark
    uint32_t receiver; //ConnectTarget, receiver mark
    uint32_t message_type; //MessageType
    uint32_t with_config; //0 - no config file, 1 - has config file
    uint32_t model_count; //0 - no model files, other - models files count
    uint32_t image_count; //0 - no image, other - image count
    uint32_t with_result; //0 - no result, 1 - has result
    struct ImageInfo image_info[IMAGE_COUNT];
    struct ResultInfo result_info;
    struct ConfigInfo config_info;
    struct ModelInfo model_info[MODEL_COUNT];
    uint8_t user_fields[USER_FIELDS_SIZE];
};

#pragma pack(pop)

#endif //BAIDU_XEYE_XEYE_MESSAGE_DATA_TYPE_H
