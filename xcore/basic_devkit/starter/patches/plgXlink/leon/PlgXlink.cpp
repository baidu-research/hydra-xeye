///
/// @file      PlgXlink.cpp
/// @copyright All code copyright Movidius Ltd 2017, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Plugin for ISP Pipelines - Xlink channel Output
///            Platform(s) supported : ma2x8x
///

#include <stdio.h>
#include <Flic.h>
#include "PlgXlink.h"
#include <ImgFrame.h>
#include <sipp.h>
#include "MsgBuf.h"
#include "BehQueueOvr.h"
#include "XLink.h"
#include "xeye_message_data_type.h"
#include "xeye_data_type.h"
#define DDR_BSS __attribute__((section(".ddr.bss")))

// LOS
//DDR_BSS struct XeyeImage g_xeye_input;
int flag_seqNo = 0;
//int g_seqNo = -1;

extern struct XeyeConfig _xeye_config;

extern __attribute__((aligned(64))) u8 DDR_BSS _message_buffer[2*XEYE_MESSAGE_SIZE];

struct XeyeMessage *g_xeye_input_message = ( XeyeMessage *)(_message_buffer);

extern int UsbSend(uint8_t* data, uint32_t size, uint32_t timeout_ms);

void sendMsg(uint8_t * p_result, uint32_t result_size)
{
    //send result back to host
    g_xeye_input_message->magic = XEYE_MAGIC;
    g_xeye_input_message->sender = CONNECTTARGET_XEYE;
    g_xeye_input_message->receiver = CONNECTTARGET_HOST;
    g_xeye_input_message->message_type = MESSAGETYPE_SET_OUTPUT;
    g_xeye_input_message->image_count = 1;
    g_xeye_input_message->with_result = 1;
    g_xeye_input_message->result_info.result_size = result_size;
    if (g_xeye_input_message->result_info.result_size <= \
            RESULT_DATA_SIZE) {
        g_xeye_input_message->result_info.embedded = 1;
        memcpy(g_xeye_input_message->result_info.result_data, \
        	   p_result, \
               g_xeye_input_message->result_info.result_size);
    } else {
        g_xeye_input_message->result_info.embedded = 0;
    }
    g_xeye_input_message->result_info.big_result_data = 0;

    g_xeye_input_message->image_info[0].image_type = IMAGETYPE_RRGGBB;
    g_xeye_input_message->image_info[0].image_data_type = IMAGEDATATYPE_UINT8;
    g_xeye_input_message->image_info[0].size = 720*1280*3/2;
    g_xeye_input_message->image_info[0].height = 720;
    g_xeye_input_message->image_info[0].width = 640;



    printf("xeye_input_message.magic:0x%04x\n",
           g_xeye_input_message->magic);
    printf("######## run_standard_mode before send.\n");

    int ret = UsbSend((uint8_t *)g_xeye_input_message,  XEYE_MESSAGE_SIZE, XEYE_VSC_TIMEOUT);

    printf("######## run_standard_mode after send.\n");
    if (0 == ret) {
        printf("send MESSAGETYPE_SET_OUTPUT successfully!\n");
    } else if (1 == ret) {
        printf("send MESSAGETYPE_SET_OUTPUT timeout.\n");
        return;
    } else {
        printf("warning: failed to send MESSAGETYPE_SET_OUTPUT. ret:%d\n", \
               ret);
        usleep(10000);
        return;
    }
}


//#############################################
void PlgXlink::Create(uint32_t maxSz, const char* channame)
{
    //Create receivers:
    in.Create(8);
    Add(&in);
    maxPktSize = maxSz;

    uint32_t strLenght = strlen(channame) + 1; /// incl termination
    if(strLenght > PLGXLINK_MAX_CHAN_NAME)
    {
        channelName[0] = 0;
        printf ( "Invalid channel name!\n" );
    }
    else
    {
        memcpy(channelName, channame, strLenght);
    }

    if(channelName[0] == 0)
    {
        printf ( "Invalid channel name!\n" );
        return;
    }

//    streamId = XLinkOpenStream(0,channelName,maxPktSize);

    printf ( "Opened xlink stream named %s\n", channelName );
}

//#############################################
void * PlgXlink::threadFunc(void *)
{
    rtems_object_set_name(RTEMS_SELF, "PlgXlink");


    while(Alive())
    {
        ImgFramePtr iFrm;
        int err = in.Receive(&iFrm);

        if(!err)
        {
            ///  Select proper size
            /// Static outputs normally have ptr->size populated correctly by pool
            /// Variable sizes, like Bitstream, have max size on ptr->size and the
            /// correct size can be calculated from framespec
            uint32_t size = 0;
            int seqNo = iFrm.ptr->seqNo;
            XLinkError_t status = X_LINK_TIMEOUT;
            switch(iFrm.ptr->fb.spec.type)
            {
                default:
                case RAW8:
                case RAW10:
                case YUV420p:
                    size = iFrm.ptr->size;
                    break;
                case BITSTREAM:
                    size = iFrm.ptr->fb.spec.width;
                    break;
            }
            if(size > 0)
            {
            	// YUV420
            	// buffer: iFrm.ptr->base
            	// size: 720*1280*3/2 = 1,382,400
            	// video channel name - vdoout
            	// nn result channel name - nnresultout
            	// TODO:

            	if(channelName[0] == 'n') // nnresultout
                {
                	// send Msg & result(embedded)
//            		g_seqNo = seqNo;

                    while(flag_seqNo!=0)
                    {
                    	usleep(1);
                    }

                    printf ( "=== 1 ==== send Msg ========, size  %d,  seq %d!\n",size, seqNo);

            		sendMsg((uint8_t*)iFrm.ptr->base, size);
                	status = X_LINK_SUCCESS;
                	flag_seqNo = 5;

                	// send result (not embedded)
            	    if (0 == g_xeye_input_message->result_info.embedded) {
            	        printf("######## run_standard_mode before send big_result.\n");

                        printf ( "=== 2 ==== send Result ========!\n" );

            	        int ret = UsbSend((uint8_t*)iFrm.ptr->base, size, XEYE_VSC_TIMEOUT);
            	        //printf("######## run_standard_mode after send big_result.\n");
            	        if (0 == ret) {
            	            printf("send big_result successfully!\n");


            	        } else if (1 == ret) {
            	            printf("send big_result timeout.\n");
            	        } else {
            	            printf("warning: failed to send big_result. ret:%d\n", ret);
            	            usleep(10000);
            	        }
            	    }

                }else if(channelName[0] == 'v')// vdoout
                {
#if 1

                    while(flag_seqNo!=5)
                    {
                    	usleep(1);
                    }
                    printf ( "=== 3 ==== send Image ========, size  %d,  seq %d!\n",size, seqNo);


                    printf ( "=== 3 ==== %d,  %d\n",g_xeye_input_message->image_count,_xeye_config.with_original_image);

                	// send image
                    if (1 == g_xeye_input_message->image_count && \
                            1 == _xeye_config.with_original_image) {
                        //send image to host
//                        image_size = lrt_g_xeye_input.org_height * lrt_g_xeye_input.org_width * 3;
//                        assert(image_size == image_info.size);
            	        int ret = UsbSend((uint8_t*)iFrm.ptr->base, size, XEYE_VSC_TIMEOUT);

                        if (0 == ret) {
                            flag_seqNo = 0;
                            printf("send image successfully!\n");
                        	status = X_LINK_SUCCESS;

                        } else if (1 == ret) {
                            printf("send image timeout.\n");
                        } else {
                            printf("warning: failed to send image. ret:%d\n", ret);
                            usleep(10000);
                        }
                    }


#endif
                }

                //XLinkError_t status = XLinkWriteData(streamId, (const uint8_t*)iFrm.ptr->base, size);
                if (status != X_LINK_SUCCESS)
                {
                    printf("myriad set data failed: %x\n", status);
                }
            }
            else
            {
                printf ( "Empty packet recieved ! Skipping..\n" );
            }
        }
    }

    return NULL;
}
