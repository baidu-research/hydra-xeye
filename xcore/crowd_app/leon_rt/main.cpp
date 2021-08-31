///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief
///

// 1: Includes
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "mv_types.h"
#include "define.h"
#include "facepub.h"
#include "LeonIPCApi.h"
#include "facemsg.h"
#include <memory>
#include "DrvCpr.h"
#include "mvLog.h"

#ifdef __cplusplus
extern "C" {
#endif
//used for calling c code
int run(deepFunc deepLearnF) {
    return runInit(deepLearnF);
}
#ifdef __cplusplus
}
#endif


#define MSG_SIZE           (sizeof(struct IpcMsg))
#define MSG_QUEUE_SIZE     32

#define ALIGNED(x) __attribute__((aligned(x)))

leonIPCChannel_t ALIGNED(4) __attribute__((section(".cmx_direct.data"))) lrt2los_channel;
uint32_t ALIGNED(4) __attribute__((section(".cmx_direct.data"))) msg_pool[MSG_QUEUE_SIZE * MSG_SIZE];
#ifdef TESTIMAGE
char ALIGNED(4) __attribute__((section(" .ddr_direct.data"))) log_buffer[log_buffer_size];
char ALIGNED(4) __attribute__((section(" .ddr_direct.data"))) frame_path[256];
#endif


int main(void) {
    int ret = 0;
    deepFunc deepLearnF = deepLearning;
    img_proc_init();

    ret = LeonIPCTxInit(&lrt2los_channel, (uint32_t*)msg_pool, MSG_QUEUE_SIZE, MSG_SIZE);
    if (ret != IPC_SUCCESS) {
        mvLog(MVLOG_FATAL, "Initialize TX IPC failed");
        return -1;
    }
    ret = run(deepLearnF);
    return ret;
}
