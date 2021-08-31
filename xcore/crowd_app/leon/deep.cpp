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

#include "mv_types.h"
#include "mvHelpersApi.h"
#include "swcCrc.h"
#include <DrvLeonL2C.h>
#include <swcShaveLoaderLocal.h>
#include <DrvShaveL2Cache.h>

#include <UnitTestApi.h>
#include <VcsHooksApi.h>

// MvTensor specific
#include "mvTensor.h"
#include "swcFrameTypes.h"
#include <swcShaveLoader.h>
#include "sipp.h"
#include "sippTestCommon.h"
#include "DrvLeon.h"
#include "resourceshare.h"
#include "define.h"
#include "pubdef.h"
#include "fathomRun.h"
#include "Fp16Convert.h"
#include "DrvTimer.h"
#include "resourceshare.h"
#include "mvTensorResize.h"
#include "facepub.h"
#include "mvTensorAffineCore.h"
#include "facemsg.h"
#include <cmath>
#include <algorithm>
#include <memory>
#include <vector>
#include "NNPreProcess.h"
#include "cam_config.h"
#include <chrono>
#include <string>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include "params/params.hpp"
#include "Log.h"
#include <mvTensorTimer.h>
#include <zbase64.h>
#include <utils/utils.hpp>
extern std::vector<t_ModelParam> lrt_g_modelParam;
extern uint64_t unix_timestamp();
extern bool lrt_fathomEndFlag;
extern DEEPLEARNING_PHASE lrt_deeplearningFlag[MAX_GRAPH];
extern uint8_t *lrt_g_pucConvUsbBuf;
extern int lrt_image_width;
extern int lrt_image_height;
extern u16 lrt_g_modelOutBuf[MAX_GRAPH][MAX_BBOX_BUF_SIZE];
extern volatile  __attribute__((section(".ddr_direct.bss"))) uint64_t lrt_g_timestamp;
extern uint8_t* lrt_g_pucConvBufY __attribute__((section(" .cmx_direct.bss")));
char tmp_nnp_buff[MAX_GRAPH][RES_WIDTH_MAX * RES_HEIGHT_MAX * 3];

int loadFrameFromNet(int sock_fd, char* buf, char* frame_path) {
    MsgHeader msg;
    msg.msg_type = REQUEST_ONE_FRAME;
    msg.content_size = 0;  // content_size is not used when msg type is request a frame
    ImgInfo img_info = {0, 0, 0, 0, 0, {0}, {0}};

    // send request to server for getting a frame
    int status = write(sock_fd, &msg, sizeof(MsgHeader));
    if (status != sizeof(MsgHeader)) {
        std::cout << "Send request failed\n";
    }
    // read header info
    status = read(sock_fd, &img_info, sizeof(img_info));
    if (status != sizeof(img_info)) {
        std::cout << "read error\n";
    }
    int size = img_info.height * img_info.width * img_info.channels;
    // if the size we computed here is 0, means this is the end of the sequence.
    if (size == 0) {
        std::cout << "end of sequence" << std::endl;
        return -1;
    }
    // input yuv format, image data size = w * h * c / 2;
    if (0 == memcmp(img_info.img_format, "yuv", 3)) {
        size /= 2;
    }

    // 64 bytes bias depends on movi_data_server
    if (frame_path) {
        memcpy(frame_path, img_info.frame_path, sizeof(img_info) - 64);
    }
    status = 0;
    do {
        status += read(sock_fd, buf + status, size - status);
    } while (status < size);
    return 0;
}

std::string get_bin_str(const char* buf_to_encode, size_t len) {
    ZBase64 base64;
    return base64.Encode((u8*)buf_to_encode, len);
}
extern void fathomrun_every (t_ModelParam * modelParam);
void deepLearning(void) {
#ifdef TESTIMAGE
    std::string server_addr = Params::getInstance()->playback_server_addr();
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(Params::getInstance()->playback_server_port());
    servaddr.sin_addr.s_addr = inet_addr(server_addr.c_str());

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // TCP ctrl client connect to server
    while (connect(sock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        std::cout << "TCP Control Client Connect to server failed, error: " << strerror(errno) << std::endl;
        usleep(100000);
    }
    extern bool lrt_end_of_sequence;
#endif

  while (1) {
      switch(lrt_deeplearningFlag[0]) {
          case EMPTY:{
              rtems_task_wake_after(1);
              break;
          }
          case DETECT:{
              extern RunningMode lrt_g_running_mode;
              if (lrt_g_running_mode == RECORD) {
                  lrt_deeplearningFlag[0] = EMPTY;
                  break;
              }
              t_ModelParam * modelParam = &lrt_g_modelParam[0];

      #ifdef TESTIMAGE
              extern char lrt_frame_path[256];
              int ret = loadFrameFromNet(sock_fd, (char*)lrt_g_pucConvUsbBuf, lrt_frame_path);
              if (ret < 0) {
                  extern bool lrt_end_of_sequence;
                  lrt_end_of_sequence = true;
                  lrt_deeplearningFlag[0] = EMPTY;
                  break;
              }
      #endif
              MovidiusImage inParams, outParams;
              inParams.nWidth  = lrt_image_width;
              inParams.nHeight = lrt_image_height;
              inParams.nFormat = IMG_FORMAT_I420;
              inParams.ws      = 0;     // Crop start X
              inParams.hs      = 0;     // Crop start Y
              inParams.we      = lrt_image_width;  // Crop end X
              inParams.he      = lrt_image_height;   // Crop end Y
              // sensor will automatically send yuv data to lrt_g_pucConvBufY
              // so in TESTIMAGE mode, we should not load image to lrt_g_pucConvBufY buffer.
              #ifdef TESTIMAGE
              inParams.pData   = (fp16*)lrt_g_pucConvUsbBuf;
              #else
              inParams.pData   = (fp16*)lrt_g_pucConvBufY;
              #endif
              outParams.nWidth     = 300;
              outParams.nHeight    = 300;
              outParams.nFormat    = IMG_FORMAT_BGR;
              outParams.mean_value[0] = 123;
              outParams.mean_value[1] = 117;
              outParams.mean_value[2] = 104;
              outParams.std_value  = 1; // Normalization value
              outParams.pData  = (fp16*)(modelParam->modelInBuf);

              NNPreProcConfig nnconfig = {
                  .dmaPriority = 1,
                  .dmaLinkAgent = 1,
              #ifdef TESTIMAGE
                  .firstShave = 0,
                  .lastShave = 5,
              #else
                  .firstShave = 4,
                  .lastShave = 7,
              #endif
                  .data_partID = 0,
                  .inst_partID = 1,
                  .tmpbuf = (u8*)tmp_nnp_buff[0]
              };
              mv::tensor::Timer deep_timer;
              float resize_time = 0.f;
              float deep_time = 0.f;
              float flag_time = 0.f;

              // Use Intel resize lib
              NNPreProcess(&inParams, &outParams, &nnconfig);
              resize_time = deep_timer.elapsed();

              // FathomRun computation
              fathomrun_every(&lrt_g_modelParam[0]);
              deep_time = deep_timer.elapsed() - resize_time;

              int num = int(f16Tof32(((fp16*)lrt_g_modelParam[0].modelOutBuf)[0]));
              if (num >= MAX_BBOX_NUM_OUTPUT) {
                  mvLog(MVLOG_ERROR, "the ouput number of bounding box exceeds %d", MAX_BBOX_NUM_OUTPUT);
                  num = MAX_BBOX_NUM_OUTPUT - 1;
              } else if (num < 0) {
                  mvLog(MVLOG_ERROR, "the output number of bounding box less than zero, \
                          usually this should not happen, but if the model loaded is not \
                          the detection model, this error will show up.");
                  num = 0;
              }
              memcpy(lrt_g_modelOutBuf[0], (fp16*)lrt_g_modelParam[0].modelOutBuf, \
                     (num + 1) * 7 * sizeof(fp16));

              flag_time = deep_timer.elapsed() - deep_time - resize_time;

              LOGD << "Preprocess takes " << resize_time
                   << "ms, deep learning takes " << deep_time
                   << "ms, till-flag-set takes: " << flag_time
                   << "ms ";
              lrt_deeplearningFlag[0] = EMPTY;
              break;
          }
          default: {
              printf("ERROR: No Such deeplearning type %d \n",lrt_deeplearningFlag[0]);
              break;
          }
      }
#ifdef TESTIMAGE
      if (lrt_end_of_sequence) {
          break;
      }
#endif
      rtems_task_wake_after(2);
    }
}
void deepLearning_multi(void) {
  while (1) {
      switch(lrt_deeplearningFlag[1]) {
          case EMPTY:{
              rtems_task_wake_after(1);
              break;
          }
          case DETECT:{
              extern RunningMode lrt_g_running_mode;
              if (lrt_g_running_mode == RECORD) {
                  lrt_deeplearningFlag[1] = EMPTY;
                  break;
              }
              t_ModelParam * modelParam = &lrt_g_modelParam[1];

              MovidiusImage inParams, outParams;
              inParams.nWidth  = lrt_image_width;
              inParams.nHeight = lrt_image_height;
              inParams.nFormat = IMG_FORMAT_I420;
              inParams.ws      = 0;     // Crop start X
              inParams.hs      = 0;     // Crop start Y
              inParams.we      = lrt_image_width;  // Crop end X
              inParams.he      = lrt_image_height;   // Crop end Y
              // sensor will automatically send yuv data to lrt_g_pucConvBufY
              // so in TESTIMAGE mode, we should not load image to lrt_g_pucConvBufY buffer.
              #ifdef TESTIMAGE
              inParams.pData   = (fp16*)lrt_g_pucConvUsbBuf;
              #else
              inParams.pData   = (fp16*)lrt_g_pucConvBufY;
              #endif
              outParams.nWidth     = 300;
              outParams.nHeight    = 300;
              outParams.nFormat    = IMG_FORMAT_BGR;
              outParams.mean_value[0] = 123;
              outParams.mean_value[1] = 117;
              outParams.mean_value[2] = 104;
              outParams.std_value  = 1; // Normalization value
              outParams.pData  = (fp16*)(modelParam->modelInBuf);

              NNPreProcConfig nnconfig = {
                  .dmaPriority = 1,
                  .dmaLinkAgent = 3,
                  .firstShave = 8,
                  .lastShave = 11,
                  .data_partID = 2,
                  .inst_partID = 3,
                  .tmpbuf = (u8*)tmp_nnp_buff[1]
              };
              mv::tensor::Timer deep_timer;
              float resize_time = 0.f;
              float deep_time = 0.f;
              float flag_time = 0.f;

              // Use Intel resize lib
              NNPreProcess(&inParams, &outParams, &nnconfig);
              resize_time = deep_timer.elapsed();

              // FathomRun computation
              fathomrun_every(&lrt_g_modelParam[1]);
              deep_time = deep_timer.elapsed() - resize_time;

              int num = int(f16Tof32(((fp16*)lrt_g_modelParam[1].modelOutBuf)[0]));
              if (num >= MAX_BBOX_NUM_OUTPUT) {
                  mvLog(MVLOG_ERROR, "the ouput number of bounding box exceeds %d", MAX_BBOX_NUM_OUTPUT);
                  num = MAX_BBOX_NUM_OUTPUT - 1;
              } else if (num < 0) {
                  mvLog(MVLOG_ERROR, "the output number of bounding box less than zero, \
                          usually this should not happen, but if the model loaded is not \
                          the detection model, this error will show up.");
                  num = 0;
              }
              memcpy(lrt_g_modelOutBuf[1], (fp16*)lrt_g_modelParam[1].modelOutBuf, \
                     (num + 1) * 7 * sizeof(fp16));

              flag_time = deep_timer.elapsed() - deep_time - resize_time;

              LOGD << "Preprocess takes " << resize_time
                   << "ms, deep learning takes " << deep_time
                   << "ms, till-flag-set takes: " << flag_time
                   << "ms ";
              //printf("t2:%f\r\n", deep_timer.elapsed());
              lrt_deeplearningFlag[1] = EMPTY;
              break;
          }
          default: {
              printf("ERROR: No Such deeplearning type %d \n",lrt_deeplearningFlag[1]);
              break;
          }
      }
      rtems_task_wake_after(2);
    }
}

