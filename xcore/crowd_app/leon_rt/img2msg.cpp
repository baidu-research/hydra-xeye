///
/// @file
/// @copyright All code copyright Movidius Ltd 2016, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     MvTensor Test application
///

// Includes
// ----------------------------------------------------------------------------
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include "mv_types.h"
#include "swcCrc.h"
#include <DrvLeonL2C.h>
#include <swcShaveLoaderLocal.h>
#include <DrvShaveL2Cache.h>

#include <UnitTestApi.h>
#include <VcsHooksApi.h>
#include "mvHelpersApi.h"

// MvTensor specific
#include "swcFrameTypes.h"
#include <swcShaveLoader.h>
#include "sipp.h"
#include "sippTestCommon.h"
#include "DrvLeon.h"
#include "define.h"
#include "pubdef.h"
#include "backproc.h"
#include "Fp16Convert.h"
#include "CamGenericApi.h"
#include "DrvTimer.h"
#include "resourceshare.h"

#include <vector>
#include <string>
#include <sstream>
#include "backproc.h"
#include "stdlib.h"
#include "DrvTimer.h"
#include "backproc.h"
#include "matching/tracker.h"
#include "datatype.h"
#include "test.h"
#include "run_tracker.h"
#include "Eigen/Core"
#include "zbase64.h"
#include "jpeg_encoder.h"
#include "mvTensorTimer.h"
#ifndef MVLOG_UNIT_NAME
#define MVLOG_UNIT_NAME img2msg
#endif

#include "mvLog.h"

using std::stringstream;

extern tracker* _this_tracker;
int jpgLen = 0;

u8 __attribute__((section(".ddr.bss"))) jpgBuf[3 * 1024 * 1024];

__attribute__((section(".ddr_direct.bss"))) char g_http_msg[4 * 1024];
char __attribute__((section(".ddr.bss"))) g_server_addr[32] = {'\0'};
char __attribute__((section(".ddr.bss"))) g_hb_server_addr[32] = {'\0'};
char __attribute__((section(".ddr.bss"))) g_server_url[64] = {'\0'};
char __attribute__((section(".ddr.bss"))) g_hb_server_url[64] = {'\0'};
volatile  __attribute__((section(".ddr.bss"))) int g_hb_server_port;
volatile  __attribute__((section(".ddr.bss"))) bool g_is_local_server;
volatile  __attribute__((section(".ddr_direct.bss"))) RunningMode g_running_mode;
volatile  __attribute__((section(".ddr_direct.bss"))) int g_server_port;
volatile  __attribute__((section(".ddr_direct.bss"))) int g_http_msg_count = 0;

// TODO(yanghongtian): refactor this interface
int get_img_jpg(int cols, int rows, const char* img) {
    mv::tensor::Timer compress_timer;
    int jpgerrcount = 0;
    do {
        jpgLen = yuvToJpeg(0, img, cols, rows, (char*)jpgBuf);
        if (jpgLen == (uint(-100))) {
            jpgerrcount++;
        }
    } while (jpgLen == (uint)(-100));
    if (jpgerrcount > 0) {
        mvLog(MVLOG_ERROR, "converting image to jpeg format error: %d", jpgerrcount);
    }
    if (jpgLen > sizeof(jpgBuf)) {
        mvLog(MVLOG_ERROR, "the size of image after compressing are too big: %d", jpgLen);
    }
    mvLog(MVLOG_DEBUG, "the size of jpg image: %d, take %f ms", \
            jpgLen, compress_timer.elapsed());
    return 0;
}

uint32_t get_img_str(void) {
    ZBase64 base64;

    if (jpgLen == 0) {
        jpgBuf[0] = 0;
        return 0;
    }

    string imgbase64 = base64.Encode(jpgBuf, jpgLen);
    memcpy(jpgBuf, imgbase64.c_str(), imgbase64.length() + 1);
    return imgbase64.length() + 1;
}
void img_proc_init(void) {

    jpgLen = 0;
    memset(jpgBuf, 0, sizeof(jpgBuf));
    memset(g_http_msg, 0, sizeof(g_http_msg));
    memset(g_server_addr, 0, sizeof(g_server_addr));
    g_server_port = 0;
    g_http_msg_count = 0;
}
