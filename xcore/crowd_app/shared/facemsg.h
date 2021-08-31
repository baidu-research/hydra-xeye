#ifndef FACEMSG_H
#define FACEMSG_H

#include <memory>
struct IpcMsg {
    int msg_type;
    size_t msg_len;
    char* msg_data;
    unsigned char* jpg_buf;
    size_t jpg_len;
};

struct OnlineGrabConf {
    enum {
        INTERVAL = 0,
        ONDEMAND,
        INVALID
    };
    bool enabled;
    int strategy;
    int value;
};

// This message types are handled in LOS's thread_ipc_handler
enum IpcMsgType {
    IpcMsgDetect = 0,
    IpcMsgTracking,
    IpcMsgImage,
    IpcMsgHeartbeat,
};

typedef struct {
    uint64_t img_ts;
    int height;
    int width;
    union {
        int channels;
        int total_size;
    };
    int next_stage;
    char img_format[4];
    char frame_path[256 - 5 * sizeof(int) - sizeof(uint64_t)];
} ImgInfo;

typedef struct http_msg_ {
    enum {
        TRACKING_MSG = 0,
        HEARTBEAT_MSG,
        IMAGE_MSG
    };
    int type;
    size_t num_boxes;
    uint64_t ts;
    char* data;
    char* jpg_img;
    size_t jpg_len;
} HttpMsg;


typedef enum {
    REQUEST_ONE_FRAME = 1000,
    FEEDBACK_DETECT_RESULT = 1001,
    FEEDBACK_TRACKING_RESULT = 1002
} MsgType;

typedef struct header {
    MsgType msg_type;
    int content_size;
} MsgHeader;


#define parse_header(usbBuf)  ((DataHeader *)usbBuf)
#define parse_trackInfo(usbBuf) (parse_header(usbBuf)->type == FACE_TRACKED ? (FaceAttr*)(usbBuf+parse_header(usbBuf)->result_offset) :NULL)
#define parse_frame(usbBuf) (parse_header(usbBuf)->type == FACE_FRAME ? (FaceFrame*)(usbBuf+parse_header(usbBuf)->result_offset) :NULL)
#define parse_img(usbBuf) ((char*)(usbBuf+parse_header(usbBuf)->image_offset))


#endif

