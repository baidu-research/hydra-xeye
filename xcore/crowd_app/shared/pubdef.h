#ifndef _PUBDEF_H_
#define _PUBDEF_H_


#define DEEP_MAX_LAYERS         512
#define HTTP_MAX_FACES          5


#define FATHOM_OUTPUT_SIZE 1048675

// usually the number of bounding box that the deep learning model ouputs is around 100
// so we config the ouput buffer to contain up to 1024 bound boxes is still some kind of reasonable.
// the perfect solution here is allocating ouput buffer dynamically,
// based on the number of ouput bounding boxes
#define MAX_BBOX_NUM_OUTPUT     1024

// each bounding box is descriped as 8 fp16s, here is the format:
// ----------------------------------------------------------------------------------------------------
// |num |type|    |    |    |    |    |    |index|score|left | top |right|bottom|    |index|score|.....
// ----------------------------------------------------------------------------------------------------
#define MAX_BBOX_BUF_SIZE      (MAX_BBOX_NUM_OUTPUT << 3)

enum DEEPLEARNING_PHASE{
    EMPTY,
    DETECT,
    TRACK,
    SCORE
};

typedef enum {
    INVALID_MODE = -1,
    RECORD = 0,
    NORMAL,
    LIVE
} RunningMode;

typedef struct {
    int32_t id;
    int32_t left;
    int32_t top;
    int32_t width;
    int32_t height;
    int32_t confirmed;
    int32_t time_since_update;
} Track_t;


#define MAX_MODEL_NAME   128
typedef void (*procModel)(uint8_t *inY,uint8_t *inU,uint8_t* inV,  int iWidth, int iHeight,void * modelParam);

typedef struct thread_heartbeat_info {
    enum {
        OS_THREAD_MONITOR = 0,
        OS_THREAD_CONSUMER,
        OS_THREAD_DEEP,
        OS_THREAD_MAIN,
        RT_THREAD_DEEP,
        THREAD_TOTAL
    };

    uint64_t previous[THREAD_TOTAL];
    uint64_t current[THREAD_TOTAL];
} ThreadHeartbeatInfo;






#endif // _PUBDEF_H_
