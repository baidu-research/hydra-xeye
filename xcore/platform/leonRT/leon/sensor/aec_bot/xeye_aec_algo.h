/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _XEYE_AEC_ALGO_H
#define _XEYE_AEC_ALGO_H
#include "xeye_info.h"

#ifdef __cplusplus
extern "C" {
#endif

void XeyeDrvAecInit(int expo, int gain, XeyeBoardInfo_t XeyeBoardInfo);
void XeyeDrvAecSetParameter(int minLuma, int maxLuma, float targetBrightness, int maxGain,
                            int minExpo, int maxExpo);
void XeyeDrvStartAec(const unsigned char* data, int width, int height);

#ifdef __cplusplus
}
#endif
#endif // _XEYE_AEC_ALGO_H
