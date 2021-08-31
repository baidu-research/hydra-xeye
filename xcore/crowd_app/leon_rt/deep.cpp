///
/// @file
/// @copyright All code copyright Movidius Ltd 2016, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     MvTensor Test application
///

// Includes
// ----------------------------------------------------------------------------
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
#include "datatype.h"
#include "test.h"
#include "run_tracker.h"
#include "img2msg.h"
#include "Eigen/Core"
#include "LeonIPCApi.h"
#include <memory>
#include "facemsg.h"
#include "mvLog.h"
#include "facepub.h"
#include "cam_config.h"
#include "matching/tracker.h"
#include "utils/jpeg_utils.hpp"
#include <mvTensorTimer.h>
#include <jpeglib.h>
#include <vector>


static std::vector<Track_t> tracking_info_to_os;

extern leonIPCChannel_t lrt2los_channel;

using std::stringstream;

typedef Eigen::Matrix<float, 1, 4, Eigen::RowMajor> DETECTBOX;
typedef Eigen::Matrix<float, -1, 4, Eigen::RowMajor> DETECTBOXSS;

volatile ALIGNED(4)  __attribute__((section(" .ddr_direct.data")))  u32 deepIndex = 0x0;
volatile ALIGNED(4)  __attribute__((section(" .ddr_direct.data")))  u32 frameUsb = 0x0;
std::vector<t_ModelParam> g_modelParam;
u16 g_modelOutBuf[MAX_GRAPH][MAX_BBOX_BUF_SIZE]  __attribute__((section(".ddr_direct.bss")));

std::vector<BBox> detectBBoxs;
std::vector<TrackPerson> trackedPersons;
extern char g_xeye_id[16];

extern vector<vector<float>> person_bbox;
extern vector<RESULT_DATA> track_result;
extern int deeplearningFlag[];
extern u16* g_pusCamBuf;
extern uint8_t* g_pucConvBufY;
extern uint8_t* g_pucConvUsbBuf;

#ifdef TESTIMAGE
extern u32 g_testimg_incnum;
#endif

float g_temp_css = 0.f;
float g_temp_mss = 0.f;
float g_temp_upa0 = 0.f;
float g_temp_upa1 = 0.f;

OnlineGrabConf online_grab_conf;
bool current_grab_state = false;

int deepcount = 0;
extern int image_width;
extern int image_height;
// TODO(yanghongtian): temporary buffer for jpg compress, unify this buffer in the future.
u8 img_buf[RES_WIDTH_MAX * RES_HEIGHT_MAX * 3 / 2];
// this timestamp value must be updated in LEON OS for every frame
volatile  __attribute__((section(".ddr_direct.bss"))) uint64_t g_timestamp;
static int use_deep_res = 0;

bool trigger_online_grab(const OnlineGrabConf& _online_grab_conf, \
        const std::vector<Track_t>& tracks, int deep_count) {
    mvLog(MVLOG_DEBUG, "enabled : %d, strategy: %d, value: %d", \
            _online_grab_conf.enabled, \
            _online_grab_conf.strategy, \
            _online_grab_conf.value);
    static uint64_t ts = 0;
    if (!_online_grab_conf.enabled) {
        return false;
    }

    // skip the first frame
    if (deep_count == 0) {
        return false;
    }
    if (_online_grab_conf.strategy == OnlineGrabConf::INTERVAL) {
        if (_online_grab_conf.value <= 0) {
            return false;
        }
        // if online grabbing is working at interval mode,
        // just evaluate the result by the deep learning counters
        return ((deep_count % _online_grab_conf.value) == 0);
    } else if (_online_grab_conf.strategy == OnlineGrabConf::ONDEMAND) {
        // if online grabbing is working at ondemand mode,
        // first to check whether or not there is already tracked bounding box,
        // if yes, then check the timestamp by the flow control value,
        // if flow control condition is satisfied, then we trigger a image grabbing.
        if (tracks.size() == 0) {
            return false;
        } else {
            if (g_timestamp - ts > _online_grab_conf.value * 1000) {
                mvLog(MVLOG_DEBUG, "timestamp diff: %llu, current %u tracked.", \
                        g_timestamp - ts, tracks.size());
                ts = g_timestamp;
                return true;
            } else {
                return false;
            }
        }
    } else {
        mvLog(MVLOG_ERROR, "Invalid strategy");
        return false;
    }
}


void backprocDetectModel(int iWidth, int iHeight) {
    t_ModelParam* modelParam = &g_modelParam[0];
    detectBBoxs.clear();

    detect_backupproc((uint16_t*)g_modelOutBuf[use_deep_res], modelParam->modelOutLen, iWidth, iHeight, detectBBoxs);
}

extern void timeprint(char* str);
extern float getTime();

void initVectors(void) {
    if (detectBBoxs.capacity() < 128) {
        detectBBoxs.reserve(128);
    }
}

void initTrackinput(void) {
    person_bbox.clear();

    for (int i = 0; i < detectBBoxs.size(); i++) {
        std::vector<float> bbox_tmp;
        bbox_tmp.push_back(detectBBoxs[i].left);
        bbox_tmp.push_back(detectBBoxs[i].top);
        bbox_tmp.push_back(detectBBoxs[i].width);
        bbox_tmp.push_back(detectBBoxs[i].height);
        bbox_tmp.push_back(detectBBoxs[i].conf);
        person_bbox.push_back(bbox_tmp);
    }
}

#ifdef TESTIMAGE
void format_bbox(const std::vector<std::vector<float>>& bboxs, char* msg, int max_len, char* frame_path) {
    if (msg == NULL) {
        mvLog(MVLOG_ERROR, "invalid message buffer");
    }
    int label = 1;
    // memset(msg, 0, max_len);
    int index = sprintf(msg, "%-96s  %2d", frame_path, int(bboxs.size()));

    for (uint32_t i = 0; i < bboxs.size(); ++i) {
        const std::vector<float> &bbox = bboxs[i];
        index += sprintf(msg + index, "    %-4d %-4d %-4d %-4d %-7.6f %2d",
                 int(bbox[0]), int(bbox[1]),
                 int(bbox[2]), int(bbox[3]),
                 bbox[4], label);
        if (index > max_len) {
            mvLog(MVLOG_FATAL, "message buffer overflow\n");
        }
    }
    // for a new line
    msg[index++] = '\n';
    // for a c-style null-terminator
    msg[index++] = '\0';
}
#endif


void pack_tracking_info_into_buffer(std::vector<Track>& tracks, \
        std::vector<Track_t>* track_buf) {
    assert(track_buf != NULL);
    track_buf->reserve(tracks.size());
    // resize 0 won't release the memory of a vector
    track_buf->resize(0);
    for (Track& track : tracks) {
        DETECTBOX bbox = track.to_tlwh();
        Track_t bounding_box = { \
            track.track_id, \
            static_cast<int32_t>(bbox(0)), \
            static_cast<int32_t>(bbox(1)), \
            static_cast<int32_t>(bbox(2)), \
            static_cast<int32_t>(bbox(3)), \
            static_cast<int32_t>(track.is_confirmed()), \
            static_cast<int32_t>(track.time_since_update)
        };
        track_buf->push_back(bounding_box);
    }
}

void schedule_deeplearning() {
#ifndef TESTIMAGE
    static int last_use = -1;
    int flag = last_use == 0 ? 1 : 0;

    while (deeplearningFlag[flag] != EMPTY) {
        DrvTimerSleepMs(1);
    }
    
    deeplearningFlag[flag] = DETECT;
    //while (deeplearningFlag[flag] != EMPTY) {
        //DrvTimerSleepMs(1);
    //}
   

    use_deep_res = flag;

    if (last_use == use_deep_res) {
        printf("..........................error\r\n");
    }

    last_use = use_deep_res;
#else
    deeplearningFlag[0] = DETECT;
    while (deeplearningFlag[0] != EMPTY) {
        DrvTimerSleepMs(1);
    }
#endif
}

void deepLearning(u8* inY, u8* inU, u8* inV, int iWidth, int iHeight) {
    extern RunningMode g_running_mode;
    uint32_t img_len = 0;
    // flow control in pure recording mode, result FPS ~= 25 / 3 = 8 FPS;
    if ((g_running_mode == RECORD) && (deepcount % 3) == 0) {
        mv::tensor::Timer jpg_timer;
        get_img_jpg(iWidth, iHeight, (char *)g_pucConvBufY);
        img_len = get_img_str();
        mvLog(MVLOG_DEBUG, "prepare jpeg buffer takes %f ms, buffer len: %d", \
                jpg_timer.elapsed(), img_len);
    }

    // first trigger deep learning process, then we can go into jpeg compression
    schedule_deeplearning();
    if (g_running_mode == NORMAL) {
        current_grab_state = trigger_online_grab(online_grab_conf, \
                tracking_info_to_os, deepcount);
        if (current_grab_state) {
            memcpy(img_buf, g_pucConvBufY, image_width * image_height * 3 / 2);
            mv::tensor::Timer compressed_timer;
            extern u8 jpgBuf[1024 * 1024];
            extern int jpgLen;
            float compressed_time = 0.f;
            float base64_time = 0.f;
            jpgLen = rt_utils::compress_yuv420p_to_jpeg(jpgBuf, img_buf, image_width, image_height, 80);
            compressed_time = compressed_timer.elapsed();
            img_len = get_img_str();
            base64_time = compressed_timer.elapsed() - compressed_time;
            mvLog(MVLOG_DEBUG, "jpeg length after compressed: %u, takes %f ms, "\
                "base64 takes %f ms, deep learning counter: %d", \
                jpgLen, compressed_time, base64_time, deepcount);
        }
    }

#ifdef TESTIMAGE
    extern bool end_of_sequence;
    if (end_of_sequence) {
        return;
    }
#endif

    // tracking or image message to OS
    extern RunningMode g_running_mode;
    extern u8 jpgBuf[];
    if (g_running_mode == NORMAL) {
        mvLog(MVLOG_DEBUG, "Sending bounding box data to OS");
        backprocDetectModel(iWidth,iHeight);
        initTrackinput();
        #ifdef TESTIMAGE
        {
            // format the detected results to a given buffer
            extern char log_buffer[log_buffer_size];
            struct IpcMsg msg;
            msg.msg_type = IpcMsgDetect;
            msg.msg_len = strlen(log_buffer);
            msg.msg_data = log_buffer;

            extern char frame_path[256];
            format_bbox(person_bbox, log_buffer, log_buffer_size, frame_path);
            int status = LeonIPCSendMessage(&lrt2los_channel, (uint32_t*)&msg);
            if (status != IPC_SUCCESS) {
                mvLog(MVLOG_ERROR, "message send failed");
            }
        }
        #endif
        run_tracker();

        extern tracker* _this_tracker;
        pack_tracking_info_into_buffer(_this_tracker->tracks, &tracking_info_to_os);

        struct IpcMsg msg = {
            IpcMsgTracking,
            tracking_info_to_os.size(),
            reinterpret_cast<char*>(tracking_info_to_os.data()),
            img_len > 0 ? reinterpret_cast<unsigned char*>(jpgBuf) : NULL,
            img_len
        };
        int status = LeonIPCSendMessage(&lrt2los_channel, (uint32_t*)&msg);
        if (status != IPC_SUCCESS) {
            mvLog(MVLOG_ERROR, "tracking message send failed, status: %d", status);
        }
    } else if (g_running_mode == RECORD) {
        if (img_len > 0) {
            mvLog(MVLOG_WARN, "Sending IpcMsgImage to OS, img_len: %d", img_len);
            struct IpcMsg msg = {
                IpcMsgImage,
                0,
                NULL,
                reinterpret_cast<char*>(jpgBuf),
                img_len
            };

            int status = LeonIPCSendMessage(&lrt2los_channel, (uint32_t*)&msg);
            if (status != IPC_SUCCESS) {
                mvLog(MVLOG_ERROR, "image message send failed, status: %d", status);
            }
        }
    }
    deepcount++;
}
