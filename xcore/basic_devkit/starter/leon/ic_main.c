///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     interface between isp base pipeline and app 3A cfg and sensor
///
///
///

// 1: Includes
// ----------------------------------------------------------------------------
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <assert.h>
#include <ipipe.h>
#include "IspCommon.h"
#include <rtems.h>
#include <mqueue.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <rtems/bspIo.h>
#include <time.h>
#include "ic_main.h"
#include <utils/mms_debug.h>
#include <utils/profile/profile.h>
#include "boardconfig.h"

// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
mmsdbg_define_variable(
        vdl_ic_main,
        DL_DEFAULT,
        0,
        "vdl_ic main",
        "Guzzi IC main"
    );
#ifdef POWER_MEASUREMENT
#define PWR_FRAME_NUMBER (30)
sem_t powerMesSem;
#endif
#define MMSDEBUGLEVEL mmsdbg_use_variable(vdl_ic_main)


// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
// Isp 3A and camera control interface callbacks
// default 3A and camera controls modules have implement this functions in order to
// have feedback from receiver image events and isp events
void inc_cam_stats_ready(void *p_prv, unsigned int seqNo, void *p_cfg_prv);
void inc_cam_capture_ready(void *p_prv, unsigned int seqNo, void *p_cfg_prv);
void inc_cam_ipipe_cfg_ready(void *p_prv, unsigned int seqNo, void *p_cfg_prv);
//void inc_cam_ipipe_buff_locked(void *p_prv, void *userData, unsigned int sourceInstance,
//                void *buffZsl, uint64_t ts, unsigned int seqNo);
void inc_cam_frame_start( void *p_prv, uint32_t sourceInstance, uint32_t seqNo,
                uint64_t ts, void *userData);
//void inc_cam_frame_line_reached(void *p_prv, uint32_t sourceInstance, uint32_t seqNo,
//                uint64_t ts, void *userData);
void inc_cam_frame_end(void *p_prv, uint32_t sourceInstance, uint32_t seqNo,
                uint64_t ts, void *userData);
void inc_cam_terminate_fr( void *p_prv, uint32_t sourceInstance, void *userData);


// 4: Static Local Data
// ----------------------------------------------------------------------------
static pthread_t eventThread;
static pthread_t eventsFlicThread;
#ifdef POWER_MEASUREMENT
static pthread_t threadMeasurement;
int pwr_frame_counter=0;
int sc;
#endif
const char*      mqForGuzziName = "mqForGuzziName";
int64_t timeDelay = 0; //TODO: add better time synchronization

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------
static void      *eventLoop(void *vCtrl);
static void *    eventsFlicFunction(void *mqName);
static mqd_t     openQue(const char *mqName);
#ifdef POWER_MEASUREMENT
void *    tempPowerReadTask(void * unused);
#endif

void startAppLevelEventsReceiver(const char* mqRecName) {
    struct sched_param param;
    pthread_attr_t attr;

    assert(0 == pthread_attr_init           (&attr));
    assert(0 == pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));
    assert(0 == pthread_attr_setschedpolicy (&attr, SCHED_RR));
    assert(0 == pthread_attr_getschedparam (&attr, &param));
    param.sched_priority = 240;
    assert(0 == pthread_attr_setschedparam (&attr, &param));

    assert(0 == pthread_create(&eventsFlicThread, &attr, eventsFlicFunction, (void*)mqRecName));
    param.sched_priority = 20;
    assert(0 == pthread_attr_setschedparam (&attr, &param));
    assert(0 == pthread_create(&eventThread, &attr, eventLoop, (void*)mqForGuzziName));

#ifdef POWER_MEASUREMENT
//    sc = sem_init(&powerMesSem , 0 , 0);
//    assert(0 == sc);
    assert(0 == pthread_create(&threadMeasurement, &attr, &tempPowerReadTask, NULL));
#endif
    assert(0 == pthread_attr_destroy   (&attr));
}

// 6: Functions Implementation
// ----------------------------------------------------------------------------
static mqd_t openQue(const char *mqName) {
    struct mq_attr m_attr;
    m_attr.mq_flags = 0;
    m_attr.mq_curmsgs = 0;
    m_attr.mq_maxmsg  = 64; // configuration params !!! TODO: !!!
    m_attr.mq_msgsize = sizeof(icEv);
    mqd_t quId     = mq_open (mqName,O_RDWR | O_CREAT, S_IRWXU | S_IRWXG, &m_attr);
    if (quId == (mqd_t)-1) {
        perror ("eventLoop : Queue creation failure");
        assert(0);
        return -1;
    }
    return quId;
}
#ifdef POWER_MEASUREMENT
void * tempPowerReadTask(void * unused){
    UNUSED(unused);
    while(1){
//        sc = sem_wait(&powerMesSem);
//        assert(0 == sc);
        sc = readTemp();
        rtems_task_wake_after(10000);
        if(sc != RTEMS_SUCCESSFUL){
            printf("Read temperature failed\n");
        }
//        sc = readPower();
//        if(sc != RTEMS_SUCCESSFUL){
//            printf("Read power failed\n");
//        }
    }
    return NULL;
}
#endif
static void * eventsFlicFunction(void *vCtrl) {
    pthread_setname_np(RTEMS_SELF, "evFlic");
    mqd_t quIdFlic     = openQue((const char*)vCtrl);
    mqd_t quIdGuzzi     = openQue(mqForGuzziName);
    while(1)  {
        icEv ev;
        int32_t sz = mq_receive (quIdFlic, (char *)&ev, sizeof(icEv), 0);
        assert(sz == sizeof(icEv));
        struct timespec ts = {.tv_sec = 0,.tv_nsec = 0,};
        if(mq_timedsend(quIdGuzzi, (const char*)&ev, sizeof(icEv), 0, &ts) != 0) {
            // error happens
            if(ETIMEDOUT == errno) {
                printf("The queue was full, Los or 3A algo is to slow or block. Messages will be skipped");
            }
            else {
                printf("errno: %d \n", errno);
                assert(0 && "Unexpected mq error.");
            }
        }
        switch (ev.ctrl) {
        case IC_EVENT_TYPE_READOUT_START:
        {
//            int64_t ts = 0;
//            getTimeStamps(&ts);
//            printk("IC_EVENT_TYPE_READOUT_START id: %lu usr:%p, ts:%lld, d:%lld, seq:%lu\n",
//                    ev.EvInfo.instId, ((icIspConfig*)ev.EvInfo.userApp01)->userData,
//                    ev.EvInfo.ts+ timeDelay, (ts-(ev.EvInfo.ts+ timeDelay)), ev.EvInfo.seqNr);
        }
        break;
        case IC_EVENT_TYPE_ISP_END:
            //printk("IC_EVENT_TYPE_ISP_END id: %lu usr:%p, ts:%lu, seq:%lu\n",
            //ev.EvInfo.instId, ((icIspConfig*)ev.EvInfo.userApp01)->userData,
            //(u32)ev.EvInfo.ts + timeDelay, ev.EvInfo.seqNr);
            break;
        case IC_EVENT_TYPE_ERROR:
            //frame is skipped because not exist isp config, in this case the userApp01 not exist
            if(IC_ERROR_SRC_MIPI_CFG_MISSING == ev.EvInfo.seqNr) {
                printk("W: Isp cfg mising: %lu \n", ev.EvInfo.instId);
            }
            else {
                if(IC_ERROR_SRC_MIPI_INTERNAL_ERROR == ev.EvInfo.seqNr) {
                    printk("W: Mipi internal error, frame skipped: %lu \n", ev.EvInfo.instId);
                }
                else {
                    if(IC_ERROR_SRC_MIPI_OUT_BUFFERS_NOT_AVAILABLE == ev.EvInfo.seqNr) {
                        printk("W: Output buffer not available: %lu \n", ev.EvInfo.instId);
                    }
                    else {
                        printk("W: %lu - %lu - %p\n",ev.EvInfo.seqNr, ev.EvInfo.instId, ev.EvInfo.userApp01);
                    }
                }
            }
            break;
        case IC_EVENT_TYPE_ZSL_LOCKED:
            break;
        default:
            break;
        }
    }
}

/*
 * ****************************************************************************
 * ** Should run at a relatively high priority so that the event queue doesn't
 * ** fill up (if it did, we would drop events).
 * ****************************************************************************
 */
static void * eventLoop(void *vCtrl) {
    pthread_setname_np(RTEMS_SELF, "evLp");
    mqd_t quId     =openQue((const char*)vCtrl);

    while(1)  {
        icEv ev;
        int32_t sz = mq_receive (quId, (char *)&ev, sizeof(icEv), 0);
        assert(sz == sizeof(icEv));
        switch (ev.ctrl) {
        case IC_EVENT_TYPE_READOUT_START:
        {
            if (((icIspConfig*)ev.EvInfo.userApp01)->userData != NULL) {
                PROFILE_ADD(
                        PROFILE_ID_LRT_START_FRAME,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData,
                        ev.EvInfo.instId
                    );
                inc_cam_frame_start(
                        NULL,
                        ev.EvInfo.instId,
                        ev.EvInfo.seqNr,
                        ev.EvInfo.ts + timeDelay,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData
                    );
                inc_cam_ipipe_cfg_ready(
                        NULL,
                        ev.EvInfo.seqNr,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData
                    );
            }
        }
            break;
        case IC_EVENT_TYPE_READOUT_END:
            if (((icIspConfig*)ev.EvInfo.userApp01)->userData != NULL) {
                PROFILE_ADD(
                        PROFILE_ID_LRT_END_FRAME,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData,
                        ev.EvInfo.instId
                    );
                inc_cam_frame_end(
                        NULL,
                        ev.EvInfo.instId,
                        ev.EvInfo.seqNr,
                        (u32)ev.EvInfo.ts + timeDelay,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData
                    );
            }
            break;
        case IC_EVENT_TYPE_ISP_START:
            if (((icIspConfig*)ev.EvInfo.userApp01)->userData != NULL) {
                PROFILE_ADD(
                        PROFILE_ID_LRT_ISP_START,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData,
                        ev.EvInfo.instId
                    );
                #if 0 /* TODO: IPIPE2 not implelmented */
                inc_cam_ipipe_cfg_ready(
                        NULL,
                        ev.u.ispEvent.seqNo,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData
                    );
                #endif

            } else {
                mmsdbg(
                        DL_ERROR,
                        "IC_EVENT_TYPE_ISP_START with userdata == NULL"
                    );
            }
#ifdef POWER_MEASUREMENT
//            pwr_frame_counter++;
//            if(pwr_frame_counter >= PWR_FRAME_NUMBER){
//                pwr_frame_counter=0;
//                sc = sem_post(&powerMesSem);
//                assert(0 == sc);
//            }
#endif
            break;
        case IC_EVENT_TYPE_ISP_END:
            if (((icIspConfig*)ev.EvInfo.userApp01)->userData != NULL) {
                PROFILE_ADD(
                        PROFILE_ID_LRT_ISP_END,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData,
                        ev.EvInfo.instId
                    );
                inc_cam_stats_ready(
                        NULL,
                        ev.EvInfo.seqNr,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData
                    );
                #if 0 /* TODO: IPIPE2 not implelmented */
                inc_cam_capture_ready(
                        NULL,
                        ev.EvInfo.seqNr,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData
                    );
                #endif
            } else {
                mmsdbg(
                        DL_ERROR,
                        "IC_EVENT_TYPE_ISP_END with userdata == NULL"
                    );
            }
            break;
        case IC_EVENT_TYPE_ERROR:
            if((((icIspConfig*)ev.EvInfo.userApp01)) == NULL) {
                //
            }
            else
            {
                PROFILE_ADD(
                        PROFILE_ID_LRT_ERROR,
                        ((icIspConfig*)ev.EvInfo.userApp01)->userData,
                        ev.EvInfo.instId
                );
                //printk("Ev Warn: %lu %p\n",ev.EvInfo.seqNr,  (((icIspConfig*)ev.EvInfo.userApp01)->userData) );
                if (((icIspConfig*)ev.EvInfo.userApp01)->userData)  {
                    if((ev.EvInfo.seqNr == IC_ERROR_SRC_MIPI_CFG_SKIPPED) ||
                            (ev.EvInfo.seqNr == IC_ERROR_SRC_MIPI_OUT_BUFFERS_NOT_AVAILABLE) ||
                            (ev.EvInfo.seqNr == IC_ERROR_SRC_MIPI_INTERNAL_ERROR)) {
                        inc_cam_terminate_fr(NULL, ev.EvInfo.instId, ((icIspConfig*)ev.EvInfo.userApp01)->userData);
                    }
                }
            }
            break;
        case IC_EVENT_TYPE_ZSL_LOCKED:
            assert(ev.EvInfo.userApp02);
            PROFILE_ADD(
                    PROFILE_ID_LRT_ZSL_LOCKED,
                    ((icIspConfig*)ev.EvInfo.userApp01)->userData,
                    ev.EvInfo.instId
            );
            inc_cam_ipipe_buff_locked(
                    NULL,
                    ((icIspConfig*)ev.EvInfo.userApp02)->userData,
                    ev.EvInfo.instId,
                    ev.EvInfo.userApp01,
                    ev.EvInfo.ts + timeDelay,
                    ev.EvInfo.seqNr);
            break;
        case IC_EVENT_TYPE_CAPTURE_MADE:
            assert(ev.EvInfo.userApp02);
            inc_cam_capture_ready(
                    NULL,
                    ev.EvInfo.seqNr,
                    ((icIspConfig*)ev.EvInfo.userApp02)->userData
                );
            break;
        default:
            printf("Ev: %lu\n",ev.ctrl); assert(0); // unknought event
            break;
        }
    }
}

// Isp upper layer can control start, stop the cams receiver, and can control the isp
// base on this functions
void los_ConfigureSource(int srcIdx, icSourceConfig *sconf, int pipeId) {
    UNUSED(pipeId);
    rtems_cache_flush_multiple_data_lines(sconf, sizeof(icSourceConfig));
    if(0 == timeDelay)
        timeDelay = icConfigureSource((icSourceInstance)srcIdx, sconf);
    else
        icConfigureSource((icSourceInstance)srcIdx, sconf);
}

void los_ipipe_LockZSL(uint32_t srcIdx) {
    PROFILE_ADD(PROFILE_ID_LOS_LOCK_ZSL, srcIdx, 0);
    icLockZSL (srcIdx, 0, IC_LOCKZSL_TS_RELATIVE);
}

void los_ipipe_TriggerCapture(void* buff, void *iconf, uint32_t srcIdx) {
    PROFILE_ADD(PROFILE_ID_LOS_TRIGGER_CAPTURE, iconf, buff);
    icTriggerCapture (srcIdx, buff, iconf, IC_CAPTURE_SEND_RAW);
}

void los_configIsp(void *iconf, int ispIdx) {
    rtems_cache_flush_multiple_data_lines(iconf, sizeof(icIspConfig));
    icSendUserData((icSourceInstance)ispIdx, (void *)iconf);
}

int los_startSource(int srcIdx) {
    icStartSource((icSourceInstance)srcIdx);
    return 0;
}

int los_stopSource(int srcIdx) {
    icStopSource((icSourceInstance)srcIdx);
    return 0;
}

void los_start(void *arg) {
    UNUSED(arg);
}

void los_stop(void) {
}
