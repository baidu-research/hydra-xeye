/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

// Includes
// ----------------------------------------------------------------------------
#include <string.h>
#include <stdio.h>
#include "mv_types.h"
#include "JpegEncoderApi.h"

#include "DrvTimer.h"
#include <swcCrc.h>
#include <DrvShaveL2Cache.h>
#include <DrvCmxDma.h>

// Components
#include <UnitTestApi.h>
#include <VcsHooksApi.h>


extern uint8_t *g_pucConvBufY;
extern uint8_t *g_pucConvBufU;
extern uint8_t *g_pucConvBufV;
// Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define INPUT_BUFF_SIZE_SHAVE 8*1024

//  Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------

frameBuffer gimginfo;

// Static Function Prototypes
// ----------------------------------------------------------------------------

// Functions Implementation
// ----------------------------------------------------------------------------

// Set values on gimginfo struct and return imputSizeCoef
// volatile ALIGNED(4)  __attribute__((section(".ddr_direct.bss)))
//__attribute__((section(".ddr_direct.bss")))  u8 buftest[1920*1080*3];
//__attribute__((section(".ddr_direct.bss")))  u8 outtest[1920*1080*3];
static float ConfigureImgFull(int yuvFormat, char *img_yuv, int width, int height)
{
    int lumaValuesPerChroma = 4;
    float inputSizeCoef = 1.5;
   	#if 0
    if(yuvFormat == JPEG_422_PLANAR)
    {
        lumaValuesPerChroma = 2;
        inputSizeCoef = 2;
    }
    if(yuvFormat == JPEG_444_PLANAR)
    {
        lumaValuesPerChroma = 1;
        inputSizeCoef = 3;
    }
#endif
#if 1
    gimginfo.p1 = (u8*) (img_yuv);
    gimginfo.p2 = (u8*) ((img_yuv) + width * height);
    gimginfo.p3 = (u8*) ((img_yuv) + width * height + width * height/lumaValuesPerChroma);
  //  gimginfo.p1 = (u8*) (buftest);
   // gimginfo.p2 = (u8*) ((buftest) + width * height);
   // gimginfo.p3 = (u8*) ((buftest) + width * height + width * height/lumaValuesPerChroma);
#else
    gimginfo.p1 = (u8*) (g_pucConvBufY);
    gimginfo.p2 = (u8*) (g_pucConvBufU);
    gimginfo.p3 = (u8*) (g_pucConvBufV);
   // gimginfo.p1 = (u8*) (g_pucConvBufY_Usb);
   // gimginfo.p2 = (u8*) (g_pucConvBufU_Usb);
   // gimginfo.p3 = (u8*) (g_pucConvBufV_Usb);
#endif
    gimginfo.spec.width = width;
    gimginfo.spec.height = height;

    return inputSizeCoef;
}
extern int indeeplearning;
u32 yuvToJpeg(int yuvFormat, const char* img, int width, int height, char* outputBuf) {
    (void)ConfigureImgFull(JPEG_420_PLANAR, img,width, height);
    while(indeeplearning){
            DrvTimerSleepMs(1);
    }
    indeeplearning = 1;
    u32 ret = JPEG_encode(gimginfo, outputBuf, 3, INPUT_BUFF_SIZE_SHAVE, JPEG_420_PLANAR);

    indeeplearning = 0;
    return ret;
}

