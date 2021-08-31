#ifndef __DEFINES__
#define __DEFINES__

#include "pubdef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*deepFunc)(u8 *inY,u8*inU,u8* inV, int iWidth, int iHeight);
void deepLearning(u8 *inY,u8*inU,u8* inV, int iWidth, int iHeight);
void faceProcModelDeep(u8 *inY,u8*inU,u8* inV, int iWidth, int iHeight, void * modelParam);
int getModelNum(void);
t_ModelParam *getModelParam(void);
void* preOperation(void* yinput,void *uinput,void *vinput,void *outBuffer);
int runInit(deepFunc deepLearnF);
void timeprint(char *str);

#ifdef __cplusplus
}
#endif

#endif // __DEFINES__

