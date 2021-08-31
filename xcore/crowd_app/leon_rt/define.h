#ifndef __DEFINES__
#define __DEFINES__

#include "pubdef.h"
#include "resourceshare.h"
#include "cam_config.h"
#include "DrvI2cDefines.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct tracker_detail {
    int left;
    int top;
    int width;
    int height;
    int trackid;
    bool confirmed;
    int time_since_update;
}tracker_detail;

typedef struct BBox {
    float left;
    float top;
    float width;
    float height;
    float conf;
    int degree;
}BBox;

typedef struct TrackPerson {
    int trackid;
    BBox bbox;
}TrackPerson;

void deepLearning(u8 *inY,u8*inU,u8* inV, int iWidth, int iHeight);
typedef void (*deepFunc)(u8 *inY,u8*inU,u8* inV, int iWidth, int iHeight);
void Face_init(void);
void img_proc_init(void);


extern int runInit(deepFunc deepLearnF);
//float timeprint(char *str);
void timeprint(char *str);
float getTime(void);
float gettimerc(void);
#ifdef __cplusplus
}
#endif

#endif // __DEFINES__

