///
/// @file      ic_main.h
/// @copyright All code copyright Movidius Ltd 2017, all rights reserved.
///            For License Warranty see: common/license.txt
/// @brief     Contains all interaction part with Ipipe.
///            This file describes all allowed message interaction with LRT Pipes side.
///

#ifndef _IC_MAIN_H_
#define _IC_MAIN_H_
#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************************************
 ~~~ Includes
 **************************************************************************************************/
#include "ipipe.h"
#define _GNU_SOURCE
#include "pthread.h"
int pthread_setname_np(pthread_t thread, const char *name);


/**************************************************************************************************
 ~~~  Exported Functions
 **************************************************************************************************/
void los_ConfigureSource(int srcIdx, icSourceConfig *sconf, int pipeId);
int  los_startSource (int srcIdx);
int  los_stopSource (int srcIdx);
void los_configIsp(void *iconf, int ispIdx);
//
void los_ipipe_LockZSL(uint32_t srcIdx);
//
void los_ipipe_TriggerCapture(void* buff, void *iconf, uint32_t srcIdx);

/**************************************************************************************************
 ~~~ Imported Function Declaration - need to be implemented in order to receive events form ipipe
**************************************************************************************************/
// !!! TODO: on integration in different specific application this functions will be replace
// with application necessary functions. And will be added other in conformity with the necessity
// as response to other events sent by Lrt side !!!
// function implemented in other module that will take in consideration Ipipe events
//void incCamSendFrameEnd(int32_t sourceInstance, int32_t seqNo, uint64_t ts);
void incCamSendFrameStart(int32_t sourceInstance, int32_t seqNo, uint64_t ts);
//void incCamIspStart(int32_t ispInstance, int32_t seqNo, uint32_t userData);
//void incCamIspEnd(int32_t ispInstance, int32_t seqNo, uint32_t userData);

extern void inc_cam_ipipe_buff_locked(void *p_prv, void *userData, unsigned int sourceInstance,
        void *buffZsl, uint64_t ts, unsigned int seqNo);
// this should have a source identification
//void inc_cam_capture_ready(void *p_prv, unsigned int seqNo, void *p_cfg_prv);


//extern int getSrcLimitsLoc(uint32_t srcId, icSourceSetup* srcSet);


void startAppLevelEventsReceiver(const char* mqRecName);

#ifdef __cplusplus
}
#endif
#endif //_IC_MAIN_H_
