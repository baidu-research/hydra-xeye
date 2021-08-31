///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Example application for initializing and using a camerastack cam
///

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <rtems.h>
#include <rtems/bspIo.h>
#include <rtems/shell.h>
#include <sys/ioctl.h>

#include <OsDrvLeonMss.h>
#include "DrvLeon.h"
#include <mv_types.h>

#include <Flic.h>
#include <MemAllocator.h>

#ifdef USE_ONE_LEON
#include "GrpIspVdoZsl1L.h"
#else
#include <FlicRmt.h>
#include <GrpIspColorVdoZsl2L_OS.h>
#endif

#include <mvnci.h>
#include <PlgNetwork.h>
#include "frmdispatch.h"
#include "PlgXlink.h"
#include "CamCtrlItf.h" // help camera control functions component
#include "boardconfig.h"
#include "ic_main.h"

#include "xeye_message_data_type.h"
#include "xeye_data_type.h"
#include "../lib/sunny_usb_api.h"
#include "cJSON.h"
#include "xeye_config.h"

#define MAX_BLOB_SIZE (18*1024*1024) // allocate 18M for now
#define MAX_USB_BUFF 5242880 // 5 M limited
//__attribute__((aligned(64))) u8 __attribute__((section(".ddr.bss"))) DetectBlob[MAX_BLOB_SIZE];
__attribute__((aligned(64))) u8 __attribute__((section(".ddr.bss"))) ClassifyBlob[MAX_BLOB_SIZE];
// 2:  Source Specific #defines and types  (typedef, enum, struct)
// --------------------------------------------------------------------------------------------------------------------
#define SSD_DIM                     300
#define MOBILENET_DIM		114

#define CAM_A_IMG_W     	(2560)//(1920)
#define CAM_A_IMG_H     	(1440)//(1080)
#define CAM_A_IMG_BITRATE   (10)
#define CAM_A_NOEXPOSURE	(1)
#define CAM_A_LANES     (2)
#define CAM_A_LANEMBPS  (2100)

#ifndef N_POOL_FRMS_RAW
#define N_POOL_FRMS_RAW    5
#endif
#ifndef N_POOL_FRMS_VDO
#define N_POOL_FRMS_VDO    3
#endif
#ifndef N_POOL_FRMS_STL
#define N_POOL_FRMS_STL    2
#endif

#define ISP_FIRST_SLICE 0
#define ISP_NROFSLICES  4

#define MVNCI_NCE_HW_ENABLE_MASK (0x3) // use both HW masks
#define MVNCI_SHAVE_ENABLE_MASK (0xfff8)//(0x03f8) // use shave 7/8/9/10 for NN
#define MVNCI_CMX_SLICE_ENABLE_MASK (0x3fff8) // the last 2 slices are reserved for CMX_OTHER/CMX_DMA

#define PLGS_SECT __attribute__((section(".cmx_direct.data")))

__attribute__((aligned(64))) u8 DDR_BSS TempFrame[CAM_A_IMG_W*CAM_A_IMG_H*3]; //Todo: Fix the hard coding.
__attribute__((aligned(64))) fp16 DDR_BSS PreProcResult[400*400*4]; //Todo: Fix the hard coding.
__attribute__((aligned(64))) u8 DDR_BSS TempFrame2[CAM_A_IMG_W*CAM_A_IMG_H*3]; //Todo: Fix the hard coding.
__attribute__((aligned(64))) fp16 DDR_BSS PreProcResult2[400*400*4]; //Todo: Fix the hard coding.

__attribute__((aligned(64))) u8 DDR_BSS Det_BSS[24*1024*1024]; //Todo: Fix the hard coding.
__attribute__((aligned(64))) u8 DDR_BSS Det_BSS2[24*1024*1024]; //Todo: Fix the hard coding.


// 4: Static Local Data
// --------------------------------------------------------------------------------------------------------------------
const char* mqRecName   = "mqRecName";
const char* xlinkChNameVDO = "vdoout";
const char* xlinkChNameNNResult = "nnresultout";

#ifdef USE_ONE_LEON
GrpIspVdoZsl       grpIspVdoZsl       PLGS_SECT;
#else
extern GrpIspColorVdoZsl2L_RT grpIspColorVdoZsl2L_RT;
GrpIspColorVdoZsl2L_OS grpIspColorVdoZsl2L_OS          PLGS_SECT;
#endif
PlgFrmDispatch  plgfrmdisp    PLGS_SECT;
PlgFrmSync  plgsyncout    PLGS_SECT;

PlgNetwork  plgNN    PLGS_SECT;
PlgPool<ImgFrame>  plgPoolNN   PLGS_SECT;

PlgNetwork  plgNN2    PLGS_SECT;
PlgPool<ImgFrame>  plgPoolNN2   PLGS_SECT;

PlgXlink plgOut PLGS_SECT;
PlgXlink plgNNOut PLGS_SECT;

Pipeline           p          PLGS_SECT;


// 5: Static Function Prototypes
// --------------------------------------------------------------------------------------
static void createPipeline();

static uint32_t xLinkReadFromChannel(void *data, const char *chName, unsigned int maxsize)
{
    int streamId = XLinkOpenStream(0, chName, maxsize);
    printf("Stream is opened\n");
    uint32_t readLength = 0;

    streamPacketDesc_t * packet;
    int status = XLinkReadData(streamId, &packet);

    if (status != X_LINK_SUCCESS) {
        printf("myriad get data failed: %x\n", status);
    }
    else
    {
        memcpy((uint8_t*)data, packet->data, packet->length);
        readLength = packet->length;
    }

   status = XLinkReleaseData(streamId);
   if (status) {
       printf("release data failed: %x\n", status);
   }
   XLinkCloseStream(streamId);
   return readLength;
}

void read_blob(u8 *blob, int max_blob_size)
{
    uint32_t offset = 0;
    uint32_t readSize = 0;
    do {
        readSize = xLinkReadFromChannel((void*)(blob + offset), "inBlob", MAX_USB_BUFF);
        printf("Read blob size: %lu\n", readSize);
        offset += readSize;
    }while(readSize >= MAX_USB_BUFF);

    assert(offset<=max_blob_size);
}

int LoadModel(ModelData &model_data) {
	//TODO:
	return 0;
}


#define XEYE_MODEL_SIZE (20 * 1024 * 1024)
#define XEYE_VSC_TIMEOUT (0)
__attribute__((aligned(64))) u8 DDR_BSS _message_buffer[2*XEYE_MESSAGE_SIZE];
__attribute__((aligned(64))) u8 DDR_BSS _model_buffer[XEYE_MODEL_SIZE];
__attribute__((aligned(64))) u8 DDR_BSS _message_data[XEYE_BIG_BUF_SIZE];

__attribute__((aligned(64))) u8 DDR_BSS _result_buffer[1 * 1024 * 1024];
uint32_t g_result_size = 10*1024;
uint32_t _model_buffer_size = XEYE_MODEL_SIZE;
uint32_t _message_data_size = XEYE_BIG_BUF_SIZE;
uint32_t g_image_size = 0;

NetworkParam g_network_param = { 	NULL,
									300,
									300,
									300,
									IMG_FORMAT_I420,
									IMG_FORMAT_BGR_CNN,
									{123,117,104},
									1
									};

struct XeyeConfig _xeye_config;

// 6: Functions Implementation
// --------------------------------------------------------------------------------------
/// ===  FUNCTION  ======================================================================
///  Name:  POSIX_Init
///  Description:  main posix init function
/// =====================================================================================
#include<VcsHooksApi.h>
extern "C" void *POSIX_Init (void *args) {
    UNUSED(args);

    pthread_setname_np(RTEMS_SELF, "main");

    // ===============  have to be in board all, bun until driver will be updated is here
    brdInit();
    if(0)  app_shell_init();

    //receive xeye message
    uint32_t timeout_ms = XEYE_VSC_TIMEOUT; //XEYE_VSC_TIMEOUT 0 / 3*1000
    static bool first_message = true;

    if (first_message) {
        timeout_ms = 0;
        first_message = false;
    }


    bool _running = true;
    while (_running) {

//		printf("######## run_computation_mode before receive.\n");
		int ret = UsbReceive(_message_buffer, XEYE_MESSAGE_SIZE, timeout_ms);
//		printf("######## run_computation_mode after receive.\n");

		if (0 == ret) {
//			printf("receive message successfully!\n");
		} else if (1 == ret) {
			printf("receive message timeout.\n");
		} else {
			printf("warning: failed to receive message. ret:%d\n", ret);
			usleep(10000);
		}

		XeyeMessage* xeye_message = (XeyeMessage*)_message_buffer;

		if (xeye_message->magic != XEYE_MAGIC) {
			printf("magic does not match.\n");
		}else
		{
//			printf("magic matches.\n");
		}
		switch (xeye_message->message_type) {

		    case MESSAGETYPE_CONFIG_XEYE: {
		        if (xeye_message->with_config != 1) {
		            printf("MESSAGETYPE_CONFIG_XEYE without config.\n");
		            break;
		        }

		        if (xeye_message->model_count != 1) {
		            printf("run_computation_mode "\
		                   "MESSAGETYPE_CONFIG_XEYE without models.\n");
		            break;
		        }

		        ModelInfo& model_info = xeye_message->model_info[0];

		        if (_model_buffer_size < model_info.model_file_size) {
		            printf("not have enough space to receive model size:%d\n",
		                   (int)model_info.model_file_size);
		            break;
		        }

		        printf("1 $$$$ config_file_size:%d\n", \
		                    (int)xeye_message->config_info.config_file_size);




		        //receive model
		        printf("######## MESSAGETYPE_CONFIG_XEYE before receive.\n");
		        ret = UsbReceive(_model_buffer, model_info.model_file_size, XEYE_VSC_TIMEOUT);
		        printf("######## MESSAGETYPE_CONFIG_XEYE after receive.\n");

		        if (0 == ret) {
		            printf("receive model successfully!\n");
		        } else if (1 == ret) {
		            printf("receive model timeout.\n");
		            break;
		        } else {
		            printf("warning: failed to receive model. ret:%d\n", ret);
		            usleep(10000);
		            break;
		        }

		        _xeye_config.xeye_mode = xeye_message->config_info.xeye_mode;

#if 1
		        //fill in process config
		        FileMap config_file_map;
		        config_file_map.map = (void*)xeye_message->config_info.config_file_data;
		        config_file_map.size = xeye_message->config_info.config_file_size;
		        int ret = parse_config_file(&config_file_map, &_xeye_config);

		        if (0 != ret) {
		            printf("failed to parse config file. ret:%d\n", ret);
		            break;
		        }


		        printf("\n ==== JSON ======\n");
		        printf("\n xeye_mode  %5d",_xeye_config.xeye_mode );
		        printf("\n en_prepro  %5d",_xeye_config.enable_preprocess );
		        printf("\n en_resize  %5d",_xeye_config.enable_resize );
		        printf("\n  resize_h  %5d",_xeye_config.resize_height );
		        printf("\n  resize_w  %5d",_xeye_config.resize_width );
		        printf("\n   en_mean  %5d",_xeye_config.enable_mean );
		        printf("\n    mean_b  %5.1f",_xeye_config.mean_b );
		        printf("\n    mean_g  %5.1f",_xeye_config.mean_g );
		        printf("\n    mean_r  %5.1f",_xeye_config.mean_r );
		        printf("\n  en_scale  %5d",_xeye_config.enable_scale );
		        printf("\n     scale  %5.1f",_xeye_config.scale );
		        printf("\n  with_img  %5d\n",_xeye_config.with_original_image );
		        printf("\n ================\n\n");

		        if(_xeye_config.enable_preprocess != 0 )
		        {
		        	if(_xeye_config.enable_resize != 0)
		        	{
		        		g_network_param.width = _xeye_config.resize_width;
		        		g_network_param.height = _xeye_config.resize_height;
		        	}

		        	if(_xeye_config.enable_mean != 0)
		        	{
		        		g_network_param.mean_value[0] = _xeye_config.mean_b;
		        		g_network_param.mean_value[1] = _xeye_config.mean_g;
		        		g_network_param.mean_value[2] = _xeye_config.mean_r;
		        	}

		        	if(_xeye_config.enable_scale != 0 && _xeye_config.scale != 0)
		        	{
		        		g_network_param.std_value = 1.0/_xeye_config.scale;
		        	}
		        }



#endif


		        //load model
		        ModelData model_data;
		        model_data.model_data = (uint8_t*)_model_buffer;
		        model_data.model_size = model_info.model_file_size;

		        ret = LoadModel(model_data);

		        if (0 != ret) {
		            printf("failed to reset model %s. size:%d ret:%d\n", \
		                   model_info.model_name, (int)model_info.model_file_size, ret);
		            break;
		        }

		        printf("reset model %s successfully.\n", model_info.model_name);

//		        _xeye_config.xeye_mode = xeye_message->config_info.xeye_mode;

		        if(_xeye_config.xeye_mode == XEYEMODE_STANDARD) {
		            printf("======== current is XEYEMODE_STANDARD.\n");
//		        	_xeye_config.with_original_image = 1;
		        }

		        if (_xeye_config.xeye_mode == XEYEMODE_COMPUTATION) {
		            printf("======== current is XEYEMODE_COMPUTATION.\n");
#ifdef POWER_MEASUREMENT
					extern void * tempPowerReadTask(void * unused);
					pthread_t threadTmpRead;
					struct sched_param param;
					pthread_attr_t attr;
					assert(0 == pthread_attr_init           (&attr));
					assert(0 == pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));
					assert(0 == pthread_attr_setschedpolicy (&attr, SCHED_RR));
					assert(0 == pthread_attr_getschedparam (&attr, &param));
					param.sched_priority = 20;
					assert(0 == pthread_attr_setschedparam (&attr, &param));
					assert(0 == pthread_create(&threadTmpRead, &attr, &tempPowerReadTask, NULL));
#endif
		        }

		        if(_xeye_config.xeye_mode == XEYEMODE_STANDARD)
		        {
					MvNCIDeclareResources(MVNCI_SHAVE_ENABLE_MASK, MVNCI_NCE_HW_ENABLE_MASK, MVNCI_CMX_SLICE_ENABLE_MASK);

					// ===========  create the pipe and the dependency
					createPipeline();

					// =============== a set of events will come from the pipe. This are receiver in a posix que
					//with same name that plgEventsRec was created
					startAppLevelEventsReceiver(mqRecName);

					void gzz_init (void);
					gzz_init();
					_running = false;

		        } else {
		        	plgNN.ComputationModeInit(_model_buffer);
		        }
		        break;
		    }
		    case MESSAGETYPE_SET_INPUT: {
		        if (_xeye_config.xeye_mode != XEYEMODE_COMPUTATION) {
		            printf("current is not XEYEMODE_COMPUTATION. "\
		                   "do not handle MESSAGETYPE_SET_INPUT.\n");
		            break;
		        }

		        if (xeye_message->image_count != 1) {
		            printf("MESSAGETYPE_SET_INPUT without image.\n");
		            break;
		        }

		        ImageInfo& image_info = xeye_message->image_info[0];

		        if (_message_data_size < image_info.size) {
		            printf("not have enough space to receive image size:%d\n",
		                   (int)image_info.size);
		            break;
		        }

		        //receive image
//		        printf("######## MESSAGETYPE_SET_INPUT before receive.\n");
		        ret = UsbReceive(_message_data, image_info.size, XEYE_VSC_TIMEOUT);
//		        printf("######## MESSAGETYPE_SET_INPUT after receive.\n");

		        if (0 == ret) {
//		            printf("receive image successfully!\n");
		        } else if (1 == ret) {
		            printf("receive image timeout.\n");
		            break;
		        } else {
		            printf("warning: failed to receive image. ret:%d\n", ret);
		            usleep(10000);
		            break;
		        }

		        //check input image
		        if (image_info.image_type != IMAGETYPE_RGBRGB) {
		            printf("image_type is not IMAGETYPE_RGBRGB.\n");
		            break;
		        }

		        if (image_info.image_data_type != IMAGEDATATYPE_FP16) {
		            printf("image_data_type is not IMAGEDATATYPE_FP16.\n");
		            break;
		        }

		        uint32_t image_size = image_info.height * image_info.width * \
		                              3  / 2;

		        if (image_size != image_info.size) {
		            printf("error: input image %d != %d\n", (int)image_size, \
		                   (int)image_info.size);
		            break;
		        }

		        g_image_size = image_info.size;

		#ifdef PRINT_DEBUG_INFO
		        struct timeval tv1;
		        gettimeofday(&tv1, NULL);
		#endif
				plgNN.ComputationModeRun(_result_buffer);
		#ifdef PRINT_DEBUG_INFO
		        struct timeval tv2;
		        gettimeofday(&tv2, NULL);
		        int duration1 = (int)(tv2.tv_sec - tv1.tv_sec) * 1000 +
		                        (tv2.tv_usec - tv1.tv_usec) / 1000;
		        printf("^^^^ fathomRun t:%d cost %d ms.\n",
		                    (int)image_info.timestamp, duration1);
		#endif

		        //send result back to host
		        xeye_message->magic = XEYE_MAGIC;
		        xeye_message->sender = CONNECTTARGET_XEYE;
		        xeye_message->receiver = CONNECTTARGET_HOST;
		        xeye_message->message_type = MESSAGETYPE_SET_OUTPUT;
		        xeye_message->image_count = 0;
		        xeye_message->with_result = 1;
		        xeye_message->image_info[0].size = 720*1280*3/2;
		        xeye_message->image_info[0].height = 720;
		        xeye_message->image_info[0].width = 320;
		        xeye_message->result_info.result_size = g_result_size * sizeof(uint16_t);
		        if (xeye_message->result_info.result_size <= RESULT_DATA_SIZE) {
		            xeye_message->result_info.embedded = 1;
		            memcpy(xeye_message->result_info.result_data, \
		            		_result_buffer, xeye_message->result_info.result_size);
		        } else {
		            xeye_message->result_info.embedded = 0;
		        }
		        xeye_message->result_info.big_result_data = 0;

//		        printf("######## MESSAGETYPE_SET_INPUT before send message.\n");

		        ret = UsbSend((uint8_t*)xeye_message, XEYE_MESSAGE_SIZE, \
		                               XEYE_VSC_TIMEOUT);

//		        printf("######## MESSAGETYPE_SET_INPUT after send message.\n");

		        if (0 == ret) {
//		            printf("send MESSAGETYPE_SET_OUTPUT successfully!\n");
		        } else if (1 == ret) {
		            printf("send MESSAGETYPE_SET_OUTPUT timeout.\n");
		            break;
		        } else {
		            printf("warning: failed to send MESSAGETYPE_SET_OUTPUT. ret:%d\n", \
		                   ret);
		            usleep(10000);
		            break;
		        }

		        if (0 == xeye_message->result_info.embedded) {
		            assert(xeye_message->result_info.result_size > RESULT_DATA_SIZE);
		            printf("######## MESSAGETYPE_SET_INPUT before send result.\n");

		            ret = UsbSend((uint8_t*)_result_buffer, \
		                                   xeye_message->result_info.result_size, \
		                                   XEYE_VSC_TIMEOUT);

		            printf("######## MESSAGETYPE_SET_INPUT after send result.\n");

		            if (0 == ret) {
		                printf("send result successfully!\n");
		            } else if (1 == ret) {
		                printf("send result timeout.\n");
		                break;
		            } else {
		                printf("warning: failed to send result. ret:%d\n", \
		                       ret);
		                usleep(10000);
		                break;
		            }
		        }

		        break;
		    }

			default:
				printf("xeye cannot handle message type %d.\n", \
					   (int)xeye_message->message_type);
				break;
			}

    }

	printf("Running loop end!\n");


    while(1)
    {
    	usleep(1000000);
    }
    return NULL;
}


// 8: Local Functions Implementation
// --------------------------------------------------------------------------------------

//
static void createPipeline() {

    GrpIspVdoZslConfig grpCfg;

#ifdef USE_ONE_LEON
#else
    FlicRmt::Init();
#endif
    RgnAlloc  .Create(RgnBuff, DEF_POOL_SZ);


    uint32_t stride =  CAM_A_IMG_W * ((CAM_A_IMG_BITRATE + 7) >> 3);
    grpCfg.camId                   = 0;
    grpCfg.spSrcCfg.nrFrmsSrc      = N_POOL_FRMS_RAW;
    grpCfg.spSrcCfg.maxImgSz       = (CAM_A_IMG_H * stride) * CAM_A_NOEXPOSURE;

    uint32_t vdoFrmSz = (CAM_A_IMG_W * CAM_A_IMG_H * 3)>>1;
    grpCfg.spIspVdoCfg.nrFrmsIsp          = N_POOL_FRMS_VDO;
    grpCfg.spIspVdoCfg.maxImgSz           = vdoFrmSz;
    grpCfg.spIspVdoCfg.downscale2xOn      = 1;
    grpCfg.spIspVdoCfg.firstCmxSliceUsed  = ISP_FIRST_SLICE;
    grpCfg.spIspVdoCfg.nrOfCmxSliceUsed   = ISP_NROFSLICES;
    grpCfg.spIspVdoCfg.outFmt             = FMT_420;

    uint32_t stlFrmSz = (CAM_A_IMG_W * CAM_A_IMG_H * 3)>>1;
    grpCfg.spIspStillCfg.nrFrmsIsp          = N_POOL_FRMS_STL;
    grpCfg.spIspStillCfg.maxImgSz           = stlFrmSz;
    grpCfg.spIspStillCfg.downscale2xOn      = 0;
    grpCfg.spIspStillCfg.firstCmxSliceUsed  = ISP_FIRST_SLICE;
    grpCfg.spIspStillCfg.nrOfCmxSliceUsed   = ISP_NROFSLICES;
    grpCfg.spIspStillCfg.outFmt             = FMT_420;

    grpCfg.mesageQueueEventName    = mqRecName;
    grpCfg.rgnAlloc                = &RgnAlloc;
    grpCfg.ipcThreadPriority = 240; // set a hi priority for very small ipc threads
#ifdef USE_ONE_LEON
    grpIspVdoZsl  .Create(&grpCfg);
#else
    grpIspColorVdoZsl2L_OS.Create(&grpCfg, &grpIspColorVdoZsl2L_RT);
#endif
    plgfrmdisp  .Create();
    plgsyncout .Create();

    plgPoolNN   .Create(&RgnAlloc, 4, 1024 + 1024*2*MAX_OBJ); //TODO: Remove hard coding of NN output size

    PlgNetworkCfg nn_cfg;
    nn_cfg.det_parm.blobFile = (void *)_model_buffer;
    nn_cfg.det_parm.dim = g_network_param.dim;
    nn_cfg.det_parm.width = g_network_param.width;
    nn_cfg.det_parm.height = g_network_param.height;
    nn_cfg.det_parm.inFormat = IMG_FORMAT_I420;
    nn_cfg.det_parm.outFormat = IMG_FORMAT_BGR_CNN;
    nn_cfg.det_parm.mean_value[0] = g_network_param.mean_value[0];
    nn_cfg.det_parm.mean_value[1] = g_network_param.mean_value[1];
    nn_cfg.det_parm.mean_value[2] = g_network_param.mean_value[2];
    nn_cfg.det_parm.std_value = g_network_param.std_value;

    nn_cfg.hw.first_shave= 4;
    nn_cfg.hw.last_shave= 7;//5;
    nn_cfg.hw.data_partID = 1;
    nn_cfg.hw.inst_partID = 0;

    nn_cfg.ddr_mem.tmpbuf = TempFrame;
    nn_cfg.ddr_mem.output = (u8 *)PreProcResult;
    plgNN .Create(&nn_cfg);
    plgNN.processingResources.first_shave = 4;
    plgNN.processingResources.last_shave = 7;//5;
    plgNN.processingResources.cnn_block = 0;
    plgNN.processingResources.first_cmx = 4;
    plgNN.processingResources.last_cmx = 7;
    plgNN.processingResources.dma_id = 0;
    plgNN.memoryResources.ddrBuffer.address = (void *)Det_BSS;
    plgNN.memoryResources.ddrBuffer.size = sizeof(Det_BSS);
    plgNN.is_config_set = 1;
    plgNN.schParam.sched_priority = 150;

    plgPoolNN2   .Create(&RgnAlloc, 4, 1024 + 1024*2*MAX_OBJ); //TODO: Remove hard coding of NN output size

    PlgNetworkCfg nn_cfg2;
    nn_cfg2.det_parm.blobFile = (void *)_model_buffer;
    nn_cfg2.det_parm.dim = g_network_param.dim;
    nn_cfg2.det_parm.width = g_network_param.width;
    nn_cfg2.det_parm.height = g_network_param.height;
    nn_cfg2.det_parm.inFormat = IMG_FORMAT_I420;
    nn_cfg2.det_parm.outFormat = IMG_FORMAT_BGR_CNN;
    nn_cfg2.det_parm.mean_value[0] = g_network_param.mean_value[0];
    nn_cfg2.det_parm.mean_value[1] = g_network_param.mean_value[1];
    nn_cfg2.det_parm.mean_value[2] = g_network_param.mean_value[2];
    nn_cfg2.det_parm.std_value = g_network_param.std_value;

    nn_cfg2.hw.first_shave= 8;
    nn_cfg2.hw.last_shave= 11;//9;
    nn_cfg2.hw.data_partID = 3;
    nn_cfg2.hw.inst_partID = 2;

    nn_cfg2.ddr_mem.tmpbuf = TempFrame2;
    nn_cfg2.ddr_mem.output = (u8 *)PreProcResult2;
    plgNN2 .Create(&nn_cfg2);
    plgNN2.processingResources.first_shave = 8;
    plgNN2.processingResources.last_shave = 11;//9;
    plgNN2.processingResources.cnn_block = 1;
    plgNN2.processingResources.first_cmx = 8;
    plgNN2.processingResources.last_cmx = 11;
    plgNN2.processingResources.dma_id = 1;
    plgNN2.memoryResources.ddrBuffer.address = (void *)Det_BSS2;
    plgNN2.memoryResources.ddrBuffer.size = sizeof(Det_BSS2);
    plgNN2.is_config_set = 1;
    plgNN2.schParam.sched_priority = 150;

    /// Xlink threads
    plgOut.Create(vdoFrmSz, xlinkChNameVDO);
    plgOut.schParam.sched_priority = FLIC_IPC_DEFAULT_THREAD_PRIORITY;

    plgNNOut.Create(vdoFrmSz, xlinkChNameNNResult);
    plgNNOut.schParam.sched_priority = FLIC_IPC_DEFAULT_THREAD_PRIORITY;


#ifdef USE_ONE_LEON
    grpIspVdoZsl.AddTo(&p);
#else
    grpIspColorVdoZsl2L_OS.AddTo(&p);
#endif
    p.Add(&plgfrmdisp);
    p.Add(&plgsyncout);
    p.Add(&plgPoolNN);
    p.Add(&plgNN);
    p.Add(&plgPoolNN2);
    p.Add(&plgNN2);
    p.Add(&plgOut);
    p.Add(&plgNNOut);

    plgPoolNN  .out       .Link(&plgNN.resultInput);
    plgPoolNN2  .out       .Link(&plgNN2.resultInput);
#ifdef USE_ONE_LEON
    grpIspVdoZsl.outVdo->Link(&plgOut.in);
#else
    grpIspColorVdoZsl2L_OS.outVdo->Link(&plgOut.in);
    grpIspColorVdoZsl2L_OS.outVdo->Link(&plgfrmdisp.inFrame);
    plgfrmdisp.outF_even.Link(&plgNN.inF);
    plgfrmdisp.outF_odd.Link(&plgNN2.inF);
#endif

    plgNN.output.Link(&plgsyncout.inFrame_even);
    plgNN2.output.Link(&plgsyncout.inFrame_odd);

    plgsyncout.out.Link(&plgNNOut.in);

#ifdef USE_ONE_LEON
#else
    DrvLeonRTStartupStart();
    DrvLeonRTWaitForBoot (); // todo: Delete while (1) approach is wrong in rtems
#endif

    p.Start();
}
