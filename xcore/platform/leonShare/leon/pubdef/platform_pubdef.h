/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _PLATFORM_PUBDEF_H_
#define _PLATFORM_PUBDEF_H_
typedef enum {
    XEYE_20 = 1,
    XEYE_FACE = 2
} XeyePcbType;

typedef struct {
    int graphLen;
    int type;
    char* graph;
} t_graph;

typedef enum {
    FACE_TYPE = 1,
    HAND_TYPE = 2,
    OBJECT_TYPE = 3,
    POSE_TYPE = 4,
    CARPLATE_TYPE = 5,
    DETECT_TYPE = 6,
    TRACK_TYPE = 7,
    SCORE_TYPE = 8,
    ATTR_TYPE = 9,
    ALIGN_TYPE = 10
} model_type_t;

typedef enum CameraMode {
    SENOSR_MODE = 0,
    USB_PLAYBACK_MODE,
    NET_PLAYBACK_MODE,
    STORAYE_PLAYBACK_MODE
} CameraMode_t;

#endif // _PLATFORM_PUBDEF_H_
