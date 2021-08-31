///
/// @file
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Camera wrapper.
///
/// This implements a wrapper over multiple drivers needed to start a sensor streaming
///
///

// 1: Includes
// ----------------------------------------------------------------------------
// Drivers includes

#include "assert.h"
#include "string.h"
#include "registersMyriad.h"
#include "DrvCif.h"
#include "DrvSipp.h"
#include "DrvMss.h"
#include "DrvGpio.h"
#include "DrvIcbDefines.h"
#include "DrvSvu.h"
#include "DrvIcb.h"
#include "DrvTimer.h"
#include "DrvMipi.h"
#include "stdlib.h"
#include "DrvCpr.h"
#include "DrvI2cMaster.h"
#include "swcWhoAmI.h"

//Project specific defines
#include "CamGenericApi.h"
#include "CamGenericPrivateDefines.h"

//#define LOCAL_DEBUG_OVERVIEW
//#define LOCAL_DEBUG_DETAILED

#ifdef LOCAL_DEBUG_OVERVIEW
#include "stdio.h"
#define DPRINTF(...) printf(__VA_ARGS__)
#else
#define DPRINTF(...)
#endif

#ifdef LOCAL_DEBUG_DETAILED
#include "stdio.h"
#define DPRINTF2(...) printf(__VA_ARGS__)
#else
#define DPRINTF2(...)
#endif

// Source Specific #defines
// ----------------------------------------------------------------------------
#define SIPP_MIPI_ALL_ID_MASK (SIPP_MIPI_RX0_ID_MASK | SIPP_MIPI_RX1_ID_MASK | SIPP_MIPI_RX2_ID_MASK | SIPP_MIPI_RX3_ID_MASK)


// Global Data
// ----------------------------------------------------------------------------
//HW descriptors for cameras
CifHwRegsAndMasks cifRegsArray[2] = {
    {
        .CIF_BASE_ADR           = CIF1_BASE_ADR,
        .INT_CLEAR_ADR          = CIF1_INT_CLEAR_ADR,
        .INT_ENABLE_ADR         = CIF1_INT_ENABLE_ADR,
        .INT_STATUS_ADR         = CIF1_INT_STATUS_ADR,
        .LINE_COUNT_ADR         = CIF1_LINE_COUNT_ADR,
        .FRAME_COUNT_ADR        = CIF1_FRAME_COUNT_ADR,
        .int_id_default         = IRQ_CIF0,
        .int_id                 = IRQ_CIF0,                          // changeable to a dynamic IRQ and back to default, at runtime, by CamSetupInterrupts()
        .int_clear_mask         = 0xFFFFFFFF,                        // changeable at runtime by CamSetupInterrupts()
        .int_management_mask    = CIF_INT_DMA0_DONE,                 // changeable at runtime by CamSetupInterrupts()
        .int_notification_mask  = 0,                                 // changeable at runtime by CamSetupInterrupts()
        .MIPI_sync              = NO_RESYNC
    },
    {
        .CIF_BASE_ADR           = CIF2_BASE_ADR,
        .INT_CLEAR_ADR          = CIF2_INT_CLEAR_ADR,
        .INT_ENABLE_ADR         = CIF2_INT_ENABLE_ADR,
        .INT_STATUS_ADR         = CIF2_INT_STATUS_ADR,
        .LINE_COUNT_ADR         = CIF2_LINE_COUNT_ADR,
        .FRAME_COUNT_ADR        = CIF2_FRAME_COUNT_ADR,
        .int_id_default         = IRQ_CIF1,
        .int_id                 = IRQ_CIF1,                          // changeable to a dynamic IRQ and back to default, at runtime, by CamSetupInterrupts()
        .int_clear_mask         = 0xFFFFFFFF,                        // changeable at runtime by CamSetupInterrupts()
        .int_management_mask    = CIF_INT_DMA0_DONE,                 // changeable at runtime by CamSetupInterrupts()
        .int_notification_mask  = 0,                                 // changeable at runtime by CamSetupInterrupts()
        .MIPI_sync              = NO_RESYNC
    }};

SippHwRegsAndMasks sippRegsArray[4] = {
    {
        .SIPP_BASE_ADR               = SIPP_MIPI_RX0_BUF_BASE_ADR,
        .LINE_COUNT_ADR              = SIPP_MIPI_RX0_FC_ADR,
        .line_int_id_default         = IRQ_SIPP_1,
        .frame_int_id_default        = IRQ_SIPP_2,
        .int_default_mask            = SIPP_MIPI_RX0_ID_MASK,
        .int_management_line_mask    = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_management_frame_mask   = SIPP_MIPI_RX0_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .int_notification_line_mask  = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_notification_frame_mask = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_clear_line_mask         = SIPP_MIPI_RX0_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .int_clear_frame_mask        = SIPP_MIPI_RX0_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .line_int_id                 = IRQ_SIPP_1,                   // changeable at runtime by CamSetupInterrupts()
        .frame_int_id                = IRQ_SIPP_2,                   // changeable at runtime by CamSetupInterrupts()
    },
    {
        .SIPP_BASE_ADR               = SIPP_MIPI_RX1_BUF_BASE_ADR,
        .LINE_COUNT_ADR              = SIPP_MIPI_RX1_FC_ADR,
        .line_int_id_default         = IRQ_SIPP_1,
        .frame_int_id_default        = IRQ_SIPP_2,
        .int_default_mask            = SIPP_MIPI_RX1_ID_MASK,
        .int_management_line_mask    = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_management_frame_mask   = SIPP_MIPI_RX1_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .int_notification_line_mask  = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_notification_frame_mask = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_clear_line_mask         = SIPP_MIPI_RX1_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .int_clear_frame_mask        = SIPP_MIPI_RX1_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .line_int_id                 = IRQ_SIPP_1,                   // changeable at runtime by CamSetupInterrupts()
        .frame_int_id                = IRQ_SIPP_2,                   // changeable at runtime by CamSetupInterrupts()
    },
    {
        .SIPP_BASE_ADR               = SIPP_MIPI_RX2_BUF_BASE_ADR,
        .LINE_COUNT_ADR              = SIPP_MIPI_RX2_FC_ADR,
        .line_int_id_default         = IRQ_SIPP_1,
        .frame_int_id_default        = IRQ_SIPP_2,
        .int_default_mask            = SIPP_MIPI_RX2_ID_MASK,
        .int_management_line_mask    = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_management_frame_mask   = SIPP_MIPI_RX2_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .int_notification_line_mask  = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_notification_frame_mask = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_clear_line_mask         = SIPP_MIPI_RX2_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .int_clear_frame_mask        = SIPP_MIPI_RX2_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .line_int_id                 = IRQ_SIPP_1,                   // changeable at runtime by CamSetupInterrupts()
        .frame_int_id                = IRQ_SIPP_2,                   // changeable at runtime by CamSetupInterrupts()
    },
    {
        .SIPP_BASE_ADR               = SIPP_MIPI_RX3_BUF_BASE_ADR,
        .LINE_COUNT_ADR              = SIPP_MIPI_RX3_FC_ADR,
        .line_int_id_default         = IRQ_SIPP_1,
        .frame_int_id_default        = IRQ_SIPP_2,
        .int_default_mask            = SIPP_MIPI_RX3_ID_MASK,
        .int_management_line_mask    = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_management_frame_mask   = SIPP_MIPI_RX3_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .int_notification_line_mask  = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_notification_frame_mask = 0,                            // changeable at runtime by CamSetupInterrupts()
        .int_clear_line_mask         = SIPP_MIPI_RX3_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .int_clear_frame_mask        = SIPP_MIPI_RX3_ID_MASK,        // changeable at runtime by CamSetupInterrupts()
        .line_int_id                 = IRQ_SIPP_1,                   // changeable at runtime by CamSetupInterrupts()
        .frame_int_id                = IRQ_SIPP_2,                   // changeable at runtime by CamSetupInterrupts()
    }
};

static GenericCameraHandle *camHndlArray[6]= {NULL, NULL, NULL, NULL, NULL, NULL};


// Functions Implementation
// ----------------------------------------------------------------------------
static eDrvMipiInitFuncRet InitMIPI(GenericCamSpec *cameraSpec, eDrvMipiCtrlNo controllerNb )
{
    eDrvMipiInitFuncRet retValue;

    tDrvMipiControllerConfigStruct mipiCtrlCfg  =
    {
        .type      =  MIPI_CSI,
        .direction =  MIPI_RX,
    };

    mipiCtrlCfg.controllerNo      = controllerNb;
    mipiCtrlCfg.rxPacket.dataMode = cameraSpec->mipiCfg->dataMode;
    mipiCtrlCfg.rxPacket.dataType = cameraSpec->mipiCfg->pixelFormat;
    mipiCtrlCfg.height            = cameraSpec->frameHeight;
    mipiCtrlCfg.width             = cameraSpec->frameWidth;
    mipiCtrlCfg.noLanes           = cameraSpec->mipiCfg->nbOflanes;
    mipiCtrlCfg.laneRateMbps      = cameraSpec->mipiCfg->dataRateMbps;

    retValue = DrvMipiInit(&mipiCtrlCfg);

    return retValue;
}


// Configuration of the sensors
static int I2CRegConfiguration(GenericCameraHandle* hndl, GenericCamSpec* camSpec, u32 startingStep, u32 nbOfSteps)
{
    static u8 camWriteProto8b[]  = {S_ADDR_WR, R_ADDR_H, R_ADDR_L, DATAW, LOOP_MINUS_1};         //register addr on 16bits, data on 8 bits
    static u8 camWriteProto16b[] = {S_ADDR_WR, R_ADDR_H, R_ADDR_L, DATAW, LOOP_MINUS_1};         //register addr on 16bits, data on 16 bits

    u8 *camWriteProto;
    unsigned int registersNb = 0, currentRegister, stepNb;
    u16 dataBytesToSend;
    u32 slaveAddr;

    DPRINTF2("Performing sensor I2C config\n");

    if ( hndl->usrSpec.stereoPairIndex == 0 )
        slaveAddr = camSpec->sensorI2CAddress1;
    else
        slaveAddr = camSpec->sensorI2CAddress2;

    if ( camSpec->regSize == 1 )
        camWriteProto = camWriteProto8b;
    else if ( camSpec->regSize == 2 )
        camWriteProto = camWriteProto16b;
    else
        return -1;

    //we know the step id but not the starting register address; pass once through all the preceding steps and calculate the start register addr
    for (stepNb = 0; stepNb < startingStep; stepNb++)
        registersNb += camSpec->i2cConfigSteps[stepNb].numberOfRegs;

    currentRegister = registersNb;

    for (stepNb = startingStep; stepNb < (startingStep + nbOfSteps); stepNb++)
    {
        DPRINTF2("\n Number of registers to configure in step %u: %ld \n", stepNb, camSpec->i2cConfigSteps[stepNb].numberOfRegs);
        DPRINTF2(" Starting register address : 0x%x\n", camSpec->regValues[currentRegister][0]);

        registersNb += camSpec->i2cConfigSteps[stepNb].numberOfRegs;

        for (; currentRegister < registersNb; currentRegister++)
        {
            dataBytesToSend = camSpec->regValues[currentRegister][1];
            if (camSpec->regSize == 2 )
                dataBytesToSend = ((dataBytesToSend & 0x00ff) << 8) | ((dataBytesToSend & 0xff00) >> 8);

            if (I2CM_STAT_OK != DrvI2cMTransaction(hndl->pI2cHandle, slaveAddr, camSpec->regValues[currentRegister][0] ,camWriteProto, (u8*)(&dataBytesToSend) , camSpec->regSize) )
            {
                DPRINTF("\n I2C sensor configuration failed at address 0x%x! Abort\n", camSpec->regValues[currentRegister][0]);
                return -1;
            }
            else
            {
                DPRINTF2("  I2C registers 0x%x set with values 0x%x\n", camSpec->regValues[currentRegister][0], dataBytesToSend);
            }
        }

        DPRINTF2(" Ending register address   : 0x%x \n", camSpec->regValues[currentRegister-1][0]);

        if ( camSpec->i2cConfigSteps[stepNb].delayMs > 0 )
            DrvTimerSleepMs(camSpec->i2cConfigSteps[stepNb].delayMs);
    }
    return 0;
}

static void CifDmaSetting(GenericCameraHandle* hndl, u32 nbOfLines)
{
    unsigned int cifBase = cifRegsArray[hndl->cifId].CIF_BASE_ADR;
    unsigned int dmaMode;

    if ((DrvCprGetCprVersionId() == HGL_VERSION_ID_MA2150_A0))
        dmaMode = D_CIF_DMA_AUTO_RESTART_DISABLED;
    else
        dmaMode = D_CIF_DMA_AUTO_RESTART_CONTINUOUS;

    switch (hndl->camSpec.internalPixelFormat)
    {
    case RAW16:
        DrvCifDma0CfgPP(cifBase,
                        (unsigned int) hndl->runningFrame->p1,
                        (unsigned int) hndl->runningFrame->p1,
                        hndl->usrSpec.windowWidth,
                        nbOfLines,
                        hndl->camSpec.bytesPerPixel,
                        dmaMode | D_CIF_DMA_AXI_BURST_8,
                        0);
        break;
    case RAW8:
        DrvCifDma0CfgPP(cifBase,
                        (unsigned int) hndl->runningFrame->p1,
                        (unsigned int) hndl->runningFrame->p1,
                        hndl->usrSpec.windowWidth,
                        nbOfLines,
                        hndl->camSpec.bytesPerPixel,
                        dmaMode | D_CIF_DMA_AXI_BURST_8,
                        0);
        break;
    case YUV420p:
        DrvCifDma0CfgPP(cifBase,
                        (unsigned int) hndl->runningFrame->p1,
                        (unsigned int) hndl->runningFrame->p1,
                        hndl->usrSpec.windowWidth,
                        nbOfLines,
                        hndl->camSpec.bytesPerPixel,
                        dmaMode | D_CIF_DMA_AXI_BURST_8,
                        0);

        DrvCifDma1CfgPP(cifBase,
                        (unsigned int) hndl->runningFrame->p2,
                        (unsigned int) hndl->runningFrame->p2,
                        hndl->usrSpec.windowWidth / 2,
                        nbOfLines / 2,
                        hndl->camSpec.bytesPerPixel,
                        dmaMode | D_CIF_DMA_AXI_BURST_8,
                        0);

        DrvCifDma2CfgPP(cifBase,
                        (unsigned int) hndl->runningFrame->p3,
                        (unsigned int) hndl->runningFrame->p3,
                        hndl->usrSpec.windowWidth / 2,
                        nbOfLines / 2,
                        hndl->camSpec.bytesPerPixel,
                        dmaMode | D_CIF_DMA_AXI_BURST_8,
                        0);
        break;
    case YUV422p:
        DrvCifDma0CfgPP(cifBase,
                        (unsigned int) hndl->runningFrame->p1,
                        (unsigned int) hndl->runningFrame->p1,
                        hndl->usrSpec.windowWidth,
                        nbOfLines,
                        hndl->camSpec.bytesPerPixel,
                        dmaMode | D_CIF_DMA_AXI_BURST_8,
                        0);

        DrvCifDma1CfgPP(cifBase,
                        (unsigned int) hndl->runningFrame->p2,
                        (unsigned int) hndl->runningFrame->p2,
                        hndl->usrSpec.windowWidth / 2,
                        nbOfLines,
                        hndl->camSpec.bytesPerPixel,
                        dmaMode | D_CIF_DMA_AXI_BURST_8,
                        0);

        DrvCifDma2CfgPP(cifBase,
                        (unsigned int) hndl->runningFrame->p3,
                        (unsigned int) hndl->runningFrame->p3,
                        hndl->usrSpec.windowWidth / 2,
                        nbOfLines,
                        hndl->camSpec.bytesPerPixel,
                        dmaMode | D_CIF_DMA_AXI_BURST_8,
                        0);
        break;
    case YUV422i:
        DrvCifDma0CfgPP(cifBase,
                        (unsigned int) hndl->runningFrame->p1,
                        (unsigned int) hndl->runningFrame->p1,
                        hndl->usrSpec.windowWidth,
                        nbOfLines,
                        hndl->camSpec.bytesPerPixel,
                        dmaMode | D_CIF_DMA_AXI_BURST_16,
                        0);
        break;
    case YUV444i :
        DrvCifDma0Cfg(cifBase,
                      (unsigned int) hndl->runningFrame->p1,
                      hndl->usrSpec.windowWidth,
                      nbOfLines,
                      hndl->camSpec.bytesPerPixel,
                      dmaMode | D_CIF_DMA_AXI_BURST_16,
                      0);
        break;
    case RGB888 :
        DrvCifDma0Cfg(cifBase,
                      (unsigned int) hndl->runningFrame->p1,
                      hndl->usrSpec.windowWidth,
                      nbOfLines,
                      hndl->camSpec.bytesPerPixel,
                      dmaMode | D_CIF_DMA_AXI_BURST_16,
                      0);
        break;
    default :
        break;
    }
    return;
}

static void CifCfg(GenericCameraHandle* hndl, unsigned int cifBase, GenericCamSpec *camSpec, u32 windowRowStart, u32 windowColumnStart, u32 window_width, u32 window_height)
{
    unsigned int outCfg;
    unsigned int outPrevCfg;
    unsigned int inputFormat;
    unsigned int outControl = 0;
    frameBuffer  dummyFrame;

    switch (hndl->camSpec.internalPixelFormat)
    {
    case YUV420p:
        outCfg = D_CIF_OUTF_FORMAT_420 | D_CIF_OUTF_CHROMA_SUB_CO_SITE_CENTER | D_CIF_OUTF_STORAGE_PLANAR | D_CIF_OUTF_Y_AFTER_CBCR;
        outPrevCfg = 0;
    	inputFormat = D_CIF_INFORM_FORMAT_YUV422 | D_CIF_INFORM_DAT_SIZE_16;
        break;
    case YUV422p:
        outCfg = D_CIF_OUTF_FORMAT_422 | D_CIF_OUTF_STORAGE_PLANAR;
        outPrevCfg = 0;
        inputFormat=D_CIF_INFORM_FORMAT_YUV422 | D_CIF_INFORM_DAT_SIZE_16;
        break;
    case YUV422i:
        outCfg =  D_CIF_OUTF_FORMAT_422;
        outPrevCfg = 0;
        inputFormat=D_CIF_INFORM_FORMAT_YUV422 | D_CIF_INFORM_DAT_SIZE_8;
        break;
    case RAW16:
        outCfg = 0; //any value here should be ignored for RAW, but unfortunately is not; it must be always 0
        outPrevCfg = D_CIF_PREV_OUTF_RGB_BY | D_CIF_PREV_RAW_BPP_2;
        inputFormat=D_CIF_INFORM_FORMAT_RGB_BAYER | D_CIF_INFORM_DAT_SIZE_16;
        outControl = D_CIF_CTRL_RGB_OUT_EN;
        break;
    case RAW8:
        outCfg = 0;  //any value here should be ignored for RAW, but unfortunately is not; it must be always 0
        outPrevCfg = D_CIF_PREV_OUTF_RGB_BY;
        inputFormat=D_CIF_INFORM_FORMAT_RGB_BAYER | D_CIF_INFORM_DAT_SIZE_8;
        outControl = D_CIF_CTRL_RGB_OUT_EN;
        break;
    case YUV444i :
        outCfg =  D_CIF_OUTF_FORMAT_444;
        outPrevCfg = 0;
        inputFormat= D_CIF_INFORM_FORMAT_YUV444 | D_CIF_INFORM_DAT_SIZE_24;
        break;
    case RGB888 :
        outCfg =  0;
        outPrevCfg = D_CIF_PREV_OUTF_RGB888 | D_CIF_PREV_BGR_ORDER;
        inputFormat= D_CIF_INFORM_FORMAT_RGB888 | D_CIF_INFORM_DAT_SIZE_24 | D_CIF_INFORM_BGR;
        outControl = D_CIF_CTRL_RGB_OUT_EN;
        break;
    default:
        outCfg =  0;
        outPrevCfg = 0;
        inputFormat=D_CIF_INFORM_FORMAT_YUV422 | D_CIF_INFORM_DAT_SIZE_8;
        break;
    }

    DrvCifTimingCfg (cifBase, camSpec->frameWidth, camSpec->frameHeight,
                     camSpec->hBackPorch, camSpec->hFrontPorch, camSpec->vBackPorch, camSpec->vFrontPorch);

    DrvCifInOutCfg (cifBase,
                    inputFormat,
                    D_CIF_IN_PIX_CLK_CPR, //0;
                    outCfg,
                    outPrevCfg );

    hndl->runningFrame     = &dummyFrame;     //dummy address here (on stack), needed for DMA initial setting; it will be anyway rewritten at CamStart
    hndl->runningFrame->p1 = NULL;            //

    if (hndl->isrFPtr.getBlock != NULL)
        CifDmaSetting(hndl, 1);
    else
        CifDmaSetting(hndl, hndl->usrSpec.windowHeight);

    DrvCifWindowing(cifBase, windowRowStart, windowColumnStart, window_width, window_height);

    if ( hndl->usrSpec.generateSync != NULL )
    {
        DrvGpioSetMode(hndl->usrSpec.generateSync->vSyncGpio, hndl->usrSpec.generateSync->vSyncGpioMode | D_GPIO_DIR_OUT);
        DrvGpioSetMode(hndl->usrSpec.generateSync->hSyncGpio, hndl->usrSpec.generateSync->hSyncGpioMode | D_GPIO_DIR_OUT);

        SET_REG_WORD(cifRegsArray[hndl->cifId].CIF_BASE_ADR + CIF_VSYNC_WIDTH_OFFSET, 16); // VSW
        SET_REG_WORD(cifRegsArray[hndl->cifId].CIF_BASE_ADR + CIF_HSYNC_WIDTH_OFFSET, 16); // HSW
        SET_REG_WORD(cifRegsArray[hndl->cifId].CIF_BASE_ADR + CIF_INPUT_IF_CFG_OFFSET, (D_CIF_IN_SINC_DRIVED_BY_CIF));

        DrvCifCtrlCfg (cifBase, camSpec->frameWidth, camSpec->frameHeight, outControl | D_CIF_CTRL_WINDOW_EN | D_CIF_CTRL_STATISTICS_FULL | D_CIF_CTRL_TIM_GEN_EN);
    }
    else
        DrvCifCtrlCfg (cifBase, camSpec->frameWidth, camSpec->frameHeight, outControl | D_CIF_CTRL_WINDOW_EN | D_CIF_CTRL_STATISTICS_FULL);
}


static u32 bitsFromMipiPixelFormat(eDrvMipiDataType pixelFormat)
{

    switch (pixelFormat)
    {
    case CSI_RAW_6:
        return 6;
    case CSI_RAW_7:
        return 7;
    case CSI_RAW_8:
        return 8;
    case CSI_RAW_10:
        return 10;
    case CSI_RAW_12:
        return 12;
    case CSI_RAW_14:
        return 14;
    case CSI_RGB_565:
        return 16;
    case CSI_RGB_888:
        return 24;
    case CSI_YUV_422_B8:
        return 16;
    default:
        assert(0);
    }
}


static void SippCfg(GenericCameraHandle* hndl, unsigned int sippId, GenericCamSpec *camSpec, u32 windowRowStart, u32 windowColumnStart, u32 window_width, u32 window_height)
{
    u32 camReceivedDataMask;
    u32 camReceivedDataBitsNb;
    u32 bitsToShift;
    u32 dummyBuf;

    sipp_buf_t sippMipiRxBuf =
    {
        .plane_stride = 0
    };

    sipp_rx_t sippMipiRxCfg =
    {
        .sel01 = 0x00000f00,
        .cfg   = 1 << 0  //SIPP clk div 2;
    };

    sippMipiRxBuf.cfg         = camSpec->bytesPerPixel << 28;  //buffer fill control mode; default NOWRAP (for frame management mode)

    if (hndl->isrFPtr.getBlock != NULL)
        sippMipiRxBuf.cfg |= 1;         //pass to wrap around mode, 1 line buffer size

    sippMipiRxBuf.line_stride = camSpec->bytesPerPixel * window_width;
    sippMipiRxBuf.base        = (u32)&dummyBuf;  //dummy address here, as it is , anyway, rewritten at CamStart and here might not be present yet

    switch ( camSpec->bytesPerPixel)
    {
    case 1:
        camReceivedDataMask = 0x000000FF;
        break;
    case 2:
        camReceivedDataMask = 0x0000FFFF;
        break;
    case 3:
        camReceivedDataMask = 0x00FFFFFF;
        break;
    case 4:
        camReceivedDataMask = 0xFFFFFFFF;
        break;
    default:
        assert(0);
    }

    camReceivedDataBitsNb = bitsFromMipiPixelFormat(camSpec->mipiCfg->pixelFormat);
    if ( camReceivedDataBitsNb > (camSpec->bytesPerPixel * 8) )
    {
        bitsToShift = camReceivedDataBitsNb - (camSpec->bytesPerPixel * 8);

        camReceivedDataMask <<= bitsToShift;
        sippMipiRxCfg.cfg |= (1 << 8) |            //enable format conversion
                        (bitsToShift << 12);  //nb. of bits to shift
    }

    sippMipiRxCfg.mask0       = camReceivedDataMask;
    sippMipiRxCfg.frm_dim     = ( (window_height << 16) | window_width);
    sippMipiRxCfg.x0          = ( (window_width << 16)  | windowColumnStart);
    sippMipiRxCfg.y0          = ( (window_height << 16) | windowRowStart);

    DrvSippConfigureMipiRxBuffers(sippId, &sippMipiRxBuf);
    DrvSippConfigureMipiRx(sippId, &sippMipiRxCfg);
}

//Configure the sensor by I2C
static camErrorType defaultSensorPowerUp(GenericCameraHandle* hndl)
{
    camErrorType retValue = CAM_SUCCESS;
    u32 resetPin = hndl->usrSpec.sensorResetPin;

    if(resetPin != 0xFF)
    {
        DrvGpioSetMode(resetPin, D_GPIO_DIR_OUT | D_GPIO_MODE_7 );

        DrvGpioSetPinLo(resetPin);
        DrvTimerSleepMs(1);
        DrvGpioSetPinHi(resetPin);
        DrvTimerSleepMs(20);  //TODO(?) configurable value
    }

    if ( (hndl->camSpec.regValues == NULL) || (hndl->camSpec.nbOfI2CConfigSteps < 3)) //at least three steps need to be defined in header file, the last two are always the sensor activation/standby (start/stop streaming)
    {
        return CAM_ERR_I2C_COMMUNICATION_ERROR;
    }

    if ( I2CRegConfiguration(hndl, &hndl->camSpec, 0, (hndl->camSpec.nbOfI2CConfigSteps - 2) ) == -1)  //starting step 0; nb. of steps less 2 (enable step / standby step)
        retValue = CAM_ERR_I2C_COMMUNICATION_ERROR;

    return retValue;
}

//Start the sensor (start streaming)
static camErrorType defaultSensorWakeup(GenericCameraHandle* hndl)
{
    camErrorType retValue = CAM_SUCCESS;

    if ( I2CRegConfiguration(hndl, &hndl->camSpec, (hndl->camSpec.nbOfI2CConfigSteps - 2), 1 ) == -1)  //starting the step N-1 (start streaming step)
        retValue = CAM_ERR_I2C_COMMUNICATION_ERROR;

    return retValue;
}

//Put the sensor in standby (stop streaming)
static camErrorType defaultSensorStandby(GenericCameraHandle* hndl)
{
    camErrorType retValue = CAM_SUCCESS;

    if ( I2CRegConfiguration(hndl, &hndl->camSpec, (hndl->camSpec.nbOfI2CConfigSteps - 1), 1 ) == -1)  //starting the last step (step N = stop streaming step)
        retValue = CAM_ERR_I2C_COMMUNICATION_ERROR;

    return retValue;
}


static camErrorType defaultSensorPowerDown(GenericCameraHandle* hndl)
{
    camErrorType retValue = CAM_SUCCESS;

    //stop streaming...
    if ( I2CRegConfiguration(hndl, &hndl->camSpec, (hndl->camSpec.nbOfI2CConfigSteps - 1), 1 ) == -1)  //starting the last step (step N = stop streaming step)
        retValue = CAM_ERR_I2C_COMMUNICATION_ERROR;

    //and reset the sensor
    if ( hndl->usrSpec.sensorResetPin != 0xFF )
        DrvGpioSetPinLo( hndl->usrSpec.sensorResetPin );

    return retValue;
}


inline static void setNextSippBufferAndRestart(frameBuffer *nextFrame, int baseAddr, int sippId)
{
    assert(nextFrame     != NULL);
    assert(nextFrame->p1 != NULL);
    //only p1 (one single plane for SIPP)
    DrvSippAddrUpdate(nextFrame->p1, baseAddr, sippId);  //address update only, no DMA engine restart (cause blocking while in overload)
}


static void setNextCifBuffersAndRestart(frameBuffer *nextFrame, int baseAddr)
{
    assert(nextFrame     != NULL);
    assert(nextFrame->p1 != NULL);

    if ((DrvCprGetCprVersionId() == HGL_VERSION_ID_MA2150_A0))
    {   //this is using one shoot mode; manual restart mandatory
        DrvCifWindowReset(baseAddr);   //Bug 22697

        DrvCifDma0RestartPP(baseAddr, (u32)nextFrame->p1, (u32)nextFrame->p1);  //address update + DMA flush/restart: needed for the DMA one shoot mode (ma2150_A0),

        if (nextFrame->p2 != NULL)
            DrvCifDma1RestartPP(baseAddr, (u32)nextFrame->p2, (u32)nextFrame->p2); //

        if (nextFrame->p3 != NULL)
            DrvCifDma2RestartPP(baseAddr, (u32)nextFrame->p3, (u32)nextFrame->p3); //
    }
    else
    {   //CIF DMA autorestart mode; address update only
        DrvCifDmaUpdate(baseAddr, CIF_DMA0, (u32)nextFrame->p1);     //address update

        if (nextFrame->p2 != NULL)
            DrvCifDmaUpdate(baseAddr, CIF_DMA1, (u32)nextFrame->p2);     //address update

        if (nextFrame->p3 != NULL)
            DrvCifDmaUpdate(baseAddr, CIF_DMA2, (u32)nextFrame->p3);     //address update
    }
}


static void CifCamHandler(u32 cifIsrSource)
{
    u32 cifCounter, cifLineCounter;
    u32 cifCamStatus;
    u32 camHndlId;
    u32 cifId = 0x00; //warning removal

    //validate the source and obtain the CIF id
    if (cifIsrSource == cifRegsArray[0].int_id)
        cifId = 0;
    else
        if (cifIsrSource == cifRegsArray[1].int_id)
            cifId = 1;
        else
        {
            assert( cifIsrSource == cifRegsArray[0].int_id ); //one of the details added, for localization
        }

    camHndlId = cifId;

    cifCamStatus = GET_REG_WORD_VAL(cifRegsArray[cifId].INT_STATUS_ADR);  //obtain the interrupt type
    cifLineCounter = GET_REG_WORD_VAL(cifRegsArray[cifId].LINE_COUNT_ADR);
    cifLineCounter &= 0xFFFF;   //remove the MSB halfword, which contains designated interrupt line for MA2x5x; TODO: upgrade will follow on this new feature

#ifdef LOCAL_DEBUG_DETAILED
    u32 cifLineBytes =  (GET_REG_WORD_VAL(cifRegsArray[cifId].CIF_BASE_ADR + CIF_DMA0_STATUS_OFFSET) % camHndlArray[camHndlId]->usrSpec.windowWidth);

    DPRINTF2("\nCIF status 0x%lx, lines %lu, bytes %ld\n", cifCamStatus, cifLineCounter, cifLineBytes);
#endif

    if ( cifCamStatus & cifRegsArray[cifId].int_management_mask )  //one single value here
    {
        if ( camHndlArray[camHndlId]->isrFPtr.getBlock != NULL ) //the test here is not about checking pointer validity but for checking which management function to call
        {
            camHndlArray[camHndlId]->frameCount = cifLineCounter;

            //the CIF generates line ISR for every line of THE SENSOR, don't matter the user window size, so a filtering of the "dummy" ISR is done below
            //in order to only call the user callbacks when within the window; TODO: protection to be removed when HW solved; also remove redundancy
            u32 lastRow;
            lastRow = camHndlArray[camHndlId]->usrSpec.windowRowStart + camHndlArray[camHndlId]->usrSpec.windowHeight;
            if ( (cifLineCounter >= camHndlArray[camHndlId]->usrSpec.windowRowStart ) && ( cifLineCounter < lastRow ) )
            {
                camHndlArray[camHndlId]->receivedFrame = camHndlArray[camHndlId]->runningFrame;
                camHndlArray[camHndlId]->runningFrame  = camHndlArray[camHndlId]->isrFPtr.getBlock(cifLineCounter);
                setNextCifBuffersAndRestart(camHndlArray[camHndlId]->runningFrame, cifRegsArray[cifId].CIF_BASE_ADR);
            }
        }
        else  //frame management
        {
            if ((cifCamStatus &  D_CIF_INT_DMA0_DONE))// && (cifLineCounter < 1070))
            {
                cifCounter = GET_REG_WORD_VAL(cifRegsArray[cifId].FRAME_COUNT_ADR);
                u32 deltaIsr = (cifCounter - camHndlArray[camHndlId]->frameCount) & 0xfff; //current ctr - previous ctr, on 12 bits
                camHndlArray[camHndlId]->frameCount = cifCounter;

                if (deltaIsr <= 1)  //CIF ISR in time for updating the address (not skipped)
                {
                    camHndlArray[camHndlId]->receivedFrame = camHndlArray[camHndlId]->runningFrame;
                }
                else //CIF ISR skipped once or more times
                {
                    camHndlArray[camHndlId]->receivedFrame = camHndlArray[camHndlId]->isrFPtr.getFrame();
                    DPRINTF(" >> CIF skipped %ld ISRs!\n", (deltaIsr-1));
                }

                camHndlArray[camHndlId]->runningFrame  = camHndlArray[camHndlId]->isrFPtr.getFrame();
                setNextCifBuffersAndRestart(camHndlArray[camHndlId]->runningFrame, cifRegsArray[cifId].CIF_BASE_ADR);
            }

            SET_REG_WORD(cifRegsArray[cifId].INT_CLEAR_ADR,  0xFFFFFFFF); //clear all

            DPRINTF2("\nCIF finished 0x%lX, running on 0x%lX\n", (u32)camHndlArray[camHndlId]->receivedFrame->p1, (u32)camHndlArray[camHndlId]->runningFrame->p1);
        }
    }

    if ( cifCamStatus & cifRegsArray[cifId].int_notification_mask )
    {
        camHndlArray[camHndlId]->isrFPtr.notification(cifCamStatus);   //compound value here; sent it, as it is, to the application, for decoding
    }
}


static void SippCamHandler(u32 sippIsrSource)
{
    u32 sippLineCnt = 0;
    u32 sippCamLineStatus, sippCamFrameStatus;
    u32 sippId, camHndlId;
    u32 exceptionalEvent = 0;

    if ( (sippIsrSource == sippRegsArray[0].line_int_id)  || (sippIsrSource == sippRegsArray[1].line_int_id)  ||
                    (sippIsrSource == sippRegsArray[2].line_int_id)  || (sippIsrSource == sippRegsArray[3].line_int_id) ||
                    (sippIsrSource == sippRegsArray[0].frame_int_id) || (sippIsrSource == sippRegsArray[1].frame_int_id) ||
                    (sippIsrSource == sippRegsArray[2].frame_int_id) || (sippIsrSource == sippRegsArray[3].frame_int_id) )
    {
        //do nothing
    }
    else
    {
        assert( sippIsrSource == sippRegsArray[0].line_int_id ); //one of the details added, for localization
    }

    sippCamLineStatus  = GET_REG_WORD_VAL(SIPP_INT1_STATUS_ADR);
    sippCamFrameStatus = GET_REG_WORD_VAL(SIPP_INT2_STATUS_ADR);

#if 0
    DPRINTF("SIPP line status:  0x%lx\n", sippCamLineStatus);
    DPRINTF("SIPP frame status: 0x%lx\n", sippCamFrameStatus);
#endif

    if ( (sippCamLineStatus & SIPP_MIPI_ALL_ID_MASK) != 0 ) //at least one SIPP MIPI line ISR has come
    {
        if ( (sippCamLineStatus & SIPP_MIPI_RX0_ID_MASK) != 0 )
        {
            camHndlId = SIPP_DEVICE0;
        }
        else if ( (sippCamLineStatus & SIPP_MIPI_RX1_ID_MASK) != 0 )
        {
            camHndlId = SIPP_DEVICE1;
        }
        else if ( (sippCamLineStatus & SIPP_MIPI_RX2_ID_MASK) != 0 )
        {
            camHndlId = SIPP_DEVICE2;
        }
        else
        {
            camHndlId = SIPP_DEVICE3;
        }

        sippId = camHndlId - SIPP_DEVICE0;

        if ((sippCamLineStatus & sippRegsArray[sippId].int_management_line_mask) != 0)
        {
            SET_REG_WORD(sippRegsArray[sippId].LINE_COUNT_ADR, 0x80000000);         //(bug) SIPP is possible blocking if OBUF register is not cleared
            camHndlArray[camHndlId]->receivedFrame = camHndlArray[camHndlId]->runningFrame;
            camHndlArray[camHndlId]->runningFrame  = camHndlArray[camHndlId]->isrFPtr.getBlock(sippLineCnt);
            setNextSippBufferAndRestart(camHndlArray[camHndlId]->runningFrame, sippRegsArray[sippId].SIPP_BASE_ADR, sippId);
        }

        if ((sippCamLineStatus & sippRegsArray[sippId].int_clear_line_mask) != 0)  //apply ISR clearing (if any) to the right SIPP filter
        {
            //in case multiple line ISRs have arrived, treat only one camera at a time, so only clear the first ISR found
            //and keep the ISR status uncleared for the remaining ones, for re-entry
            SET_REG_WORD(SIPP_INT1_CLEAR_ADR, sippRegsArray[sippId].int_clear_line_mask);
            GET_REG_WORD_VAL(SIPP_INT1_STATUS_ADR);                                        //dummy read in order to guarantee the transaction order, in case of LOS
            DrvIcbIrqClear(sippRegsArray[sippId].line_int_id);
        }

        if ((sippCamLineStatus & sippRegsArray[sippId].int_notification_line_mask) != 0)
        {
            camHndlArray[camHndlId]->isrFPtr.notification(SIPP_INT_LINE_DMA_DONE);  //translate from real sensor id to a conventional sensor status
        }
    }
    else
    {
        //capturing another SIPP block ISR in this routine? ignore it
        DPRINTF("Unknown SIPP ID in line ISR : %lx!\n", sippCamLineStatus);
    }

    if ( (sippCamFrameStatus & SIPP_MIPI_ALL_ID_MASK) != 0 ) //at least one SIPP MIPI frame ISR has come
    {
        if ( (sippCamFrameStatus & SIPP_MIPI_RX0_ID_MASK) != 0 )
        {
            camHndlId = SIPP_DEVICE0;
        }
        else if ( (sippCamFrameStatus & SIPP_MIPI_RX1_ID_MASK) != 0 )
        {
            camHndlId = SIPP_DEVICE1;
        }
        else if ( (sippCamFrameStatus & SIPP_MIPI_RX2_ID_MASK) != 0 )
        {
            camHndlId = SIPP_DEVICE2;
        }
        else
        {
            camHndlId = SIPP_DEVICE3;
        }

        sippId = camHndlId - SIPP_DEVICE0;

        if ((sippCamFrameStatus & sippRegsArray[sippId].int_management_frame_mask) != 0)
        {
            //(no frame counter available for read)
                            if ( ( (GET_REG_WORD_VAL(SIPP_STATUS_ADR)) & sippRegsArray[sippId].int_management_frame_mask) == 0 )
                            {
                                camHndlArray[camHndlId]->receivedFrame = camHndlArray[camHndlId]->runningFrame;
                                camHndlArray[camHndlId]->runningFrame  = camHndlArray[camHndlId]->isrFPtr.getFrame();    //choose the next buffer(s)
                                setNextSippBufferAndRestart(camHndlArray[camHndlId]->runningFrame, sippRegsArray[sippId].SIPP_BASE_ADR, sippId);
                            }
                            else
                            {
                                //this ISR is serviced too late, a new transaction started and is ongoing on the old buffer, so don't bother to update the address,
                                //it will create frame tearing (and possibly block the SIPP HW);
                                //instead, will have the drawback of frame lost (still image)
                                //it worth anyway notifying this event to the application
                                exceptionalEvent = SIPP_INT_FRAME_LOST;
                            }
        }


        if ((sippCamFrameStatus & sippRegsArray[sippId].int_clear_frame_mask) != 0)
        {
            //in case multiple frame ISRs have arrived, treat only one camera at a time, the first one detected, so stream down the current ISR treatment
            //to one only and keep the ISR status uncleared for the remaining ones, for re-entry
            SET_REG_WORD(SIPP_INT2_CLEAR_ADR, sippRegsArray[sippId].int_clear_frame_mask);
            GET_REG_WORD_VAL(SIPP_INT2_STATUS_ADR);                                        //dummy read in order to guarantee the transaction order, in case of LOS
            DrvIcbIrqClear(sippRegsArray[sippId].frame_int_id);
        }

        if ((sippCamFrameStatus & sippRegsArray[sippId].int_notification_frame_mask) != 0)
        {
            camHndlArray[camHndlId]->isrFPtr.notification(SIPP_INT_FRAME_DMA_DONE | exceptionalEvent);  //translate from real sensor status id to a conventional sensor status
        }
    }
    else
    {
        //capturing another SIPP frame ISR in this routine? ignore it
        DPRINTF("Unknown SIPP ID in frame ISR : %lx!\n", sippCamFrameStatus);

        return;
    }
}


extern camErrorType CamSetupCallbacks(GenericCameraHandle* hndl, sensorCallbacksListType* cbList)
{
    camErrorType retValue = CAM_SUCCESS;

    hndl->sensorFPtr.sensorPowerUp    = cbList->sensorPowerUp;
    hndl->sensorFPtr.sensorStandby    = cbList->sensorStandby;
    hndl->sensorFPtr.sensorWakeup     = cbList->sensorWakeup;
    hndl->sensorFPtr.sensorPowerDown  = cbList->sensorPowerDown;

    return (retValue);
}


camErrorType CamSetupInterrupts(GenericCameraHandle* hndl, camIsrType managedInterrupt, u32 notifiedInterrupts, u32 clearedInterrupts,
                                interruptsCallbacksListType* cbList, u32 interruptsLevel, u32 routeLineInterrupt, u32 routeFrameInterrupt )
{
    camErrorType retValue = CAM_SUCCESS;

    if (hndl == NULL)
        return (CAM_ERR_PARAMETERS_LIST_INCOMPLETE);

    if (cbList != NULL)
    {
        //store the ISR callbacks list
        hndl->isrFPtr.getBlock         = cbList->getBlock;
        hndl->isrFPtr.getFrame         = cbList->getFrame;
        hndl->isrFPtr.notification     = cbList->notification;
    }
    else
    {
        hndl->isrFPtr.getBlock         = NULL;
        hndl->isrFPtr.getFrame         = NULL;
        hndl->isrFPtr.notification     = NULL;
    }

    //validate the callbacks list
    if ( managedInterrupt != NO_CAM_ISR ) //if CamGeneric will have to manage the ISRs
    {
        if ( (hndl->isrFPtr.getFrame == NULL) && ( hndl->isrFPtr.getBlock == NULL ) )  //one and only one buffer allocation function is mandatory to exist
            return CAM_ERR_PARAMETERS_LIST_INCOMPLETE;
        if ( (hndl->isrFPtr.getFrame != NULL) && ( hndl->isrFPtr.getBlock != NULL ) )
            return CAM_ERR_CONFLICTING_ALLOC_FUNCTIONS;
    }

    if ( (notifiedInterrupts != NO_CAM_ISR) && (hndl->isrFPtr.notification == NULL) ) //if CamGeneric will have to send ISRs notification, a cbf must be provided
        return CAM_ERR_PARAMETERS_LIST_INCOMPLETE;

    //set the management / notification / clearing masks
    if ( hndl->receiverType == CIF_TYPE )
    {
        //set all the masks in one shoot
        cifRegsArray[hndl->cifId].int_management_mask   = (u32)managedInterrupt; //direct assignment for CIF

        cifRegsArray[hndl->cifId].int_notification_mask = notifiedInterrupts;    //

        cifRegsArray[hndl->cifId].int_clear_mask        = clearedInterrupts;     //
    }
    else  //SIPP
    {
        //set the managed interrupt mask
        if ( managedInterrupt == SIPP_INT_LINE_DMA_DONE )
        {
            sippRegsArray[hndl->sippId].int_management_line_mask  = sippRegsArray[hndl->sippId].int_default_mask;  //set the line notification mask and clear the frame mask
            sippRegsArray[hndl->sippId].int_management_frame_mask = 0;
        }
        else if ( managedInterrupt == SIPP_INT_FRAME_DMA_DONE )
        {
            sippRegsArray[hndl->sippId].int_management_frame_mask = sippRegsArray[hndl->sippId].int_default_mask;  //set the frame notification mask and clear the line mask
            sippRegsArray[hndl->sippId].int_management_line_mask  = 0;
        }
        else if ( managedInterrupt == NO_CAM_ISR )
        {
            sippRegsArray[hndl->sippId].int_management_line_mask  = 0;  //clear the current one
            sippRegsArray[hndl->sippId].int_management_frame_mask = 0;  //
        }
        else
        {
            return CAM_ERR_FEATURE_NOT_AVAILABLE_FOR_CURRENT_CONFIGURATION;
        }

        //set the notification interrupts mask
        sippRegsArray[hndl->sippId].int_notification_line_mask  = 0;
        sippRegsArray[hndl->sippId].int_notification_frame_mask = 0;
        if ( (notifiedInterrupts & SIPP_INT_LINE_DMA_DONE) != 0 )
        {
            sippRegsArray[hndl->sippId].int_notification_line_mask  = sippRegsArray[hndl->sippId].int_default_mask;  //set the line notification mask and clear the frame mask
        }
        if ( (notifiedInterrupts & SIPP_INT_FRAME_DMA_DONE) != 0 )
        {
            sippRegsArray[hndl->sippId].int_notification_frame_mask = sippRegsArray[hndl->sippId].int_default_mask;  //set the frame notification mask and clear the line mask
        }

        //set the clearing mask
        sippRegsArray[hndl->sippId].int_clear_line_mask  = 0;
        sippRegsArray[hndl->sippId].int_clear_frame_mask = 0;

        if ( (clearedInterrupts & SIPP_INT_LINE_DMA_DONE) != 0 )
        {
            sippRegsArray[hndl->sippId].int_clear_line_mask  = sippRegsArray[hndl->sippId].int_default_mask;
        }
        if ( (clearedInterrupts & SIPP_INT_FRAME_DMA_DONE) != 0 )
        {
            sippRegsArray[hndl->sippId].int_clear_frame_mask = sippRegsArray[hndl->sippId].int_default_mask;
        }
    }

    hndl->interruptsPriority = interruptsLevel;

    //set the interrupt(s) id, either with a customized rerouting id, or with the default HW id
    if ( hndl->receiverType == CIF_TYPE )
    {
        if (routeLineInterrupt == routeFrameInterrupt )  //for CIF, line ISR id must have same value as frame ISR id: only one ISR presented for both events
        {
            if ( (routeLineInterrupt >= IRQ_DYNAMIC_0 ) && (routeLineInterrupt <= IRQ_DYNAMIC_11) )
            {
                cifRegsArray[hndl->cifId].int_id = routeLineInterrupt;
            }
            else if (routeLineInterrupt == 0 ) //or maybe consider the 0xFF as default, as the IRQ 0 is already taken by watchdog (not bothering)
            {
                cifRegsArray[hndl->cifId].int_id = cifRegsArray[hndl->cifId].int_id_default;  //restore the default non routed ISR;
            }
            else
                return CAM_ERR_ROUTED_INTERRUPT_NOT_VALID;
        }
        else
        {
            return CAM_ERR_FEATURE_NOT_AVAILABLE_FOR_CURRENT_CONFIGURATION;
        }
    }
    else  //for SIPP, there are two distinct events that can be customized
    {
        //line ISR id
        if ( (routeLineInterrupt >= IRQ_DYNAMIC_0 ) && (routeLineInterrupt <= IRQ_DYNAMIC_11) )
        {
            sippRegsArray[hndl->sippId].line_int_id = routeLineInterrupt;
        }
        else if (routeLineInterrupt == 0 ) //or maybe consider the 0xFF as default, as the IRQ 0 is already taken by watchdog (not bothering)
        {
            sippRegsArray[hndl->sippId].line_int_id = sippRegsArray[hndl->sippId].line_int_id_default;  //restore the default non routed ISR;
        }
        else
            return CAM_ERR_ROUTED_INTERRUPT_NOT_VALID;

        //frame ISR id
        if ( (routeFrameInterrupt >= IRQ_DYNAMIC_0 ) && (routeFrameInterrupt <= IRQ_DYNAMIC_11) )
        {
            sippRegsArray[hndl->sippId].frame_int_id = routeFrameInterrupt;
        }
        else if (routeFrameInterrupt == 0 ) //or maybe consider the 0xFF as default, as the IRQ 0 is already taken by watchdog (not bothering)
        {
            sippRegsArray[hndl->sippId].frame_int_id = sippRegsArray[hndl->sippId].frame_int_id_default;  //restore the default non routed ISR;
        }
        else
            return CAM_ERR_ROUTED_INTERRUPT_NOT_VALID;
    }

    return retValue;
}


extern camErrorType CamInit(GenericCameraHandle *hndl, GenericCamSpec *camSpec, CamUserSpec *userSpec, callbacksListStruct* cbList, I2CM_Device *pI2cHandle)
{
    camErrorType retValue = CAM_SUCCESS;
    u32 isrManagement, isrNotification;
    eDrvMipiInitFuncRet mipiRetValue;

    if ( (hndl == NULL) || (camSpec == NULL) || (userSpec == NULL) )
        return (CAM_ERR_PARAMETERS_LIST_INCOMPLETE);

    if (pI2cHandle == NULL)
    {
        if ( cbList == NULL)
            return (CAM_ERR_PARAMETERS_LIST_INCOMPLETE);
        else
            if (cbList->sensorCbfList == NULL)  //the default sensor functions need the I2C handler; if no default func. used, then custom initialization functions are needed
                return (CAM_ERR_PARAMETERS_LIST_INCOMPLETE);  //TODO: check ALL the custom ptrs to be non - null !
    }
    switch (userSpec->receiverId)
    {
    case CIF_DEVICE0:
    case CIF_DEVICE1:
        hndl->receiverType = CIF_TYPE;
        break;
    case SIPP_DEVICE0:
    case SIPP_DEVICE1:
    case SIPP_DEVICE2:
    case SIPP_DEVICE3:
        hndl->receiverType = SIPP_TYPE;
        break;
    default:
        return (CAM_ERR_RECEIVER_ID_INCORRECT);
    }

    // Initialize the handle

    hndl->receiverId = userSpec->receiverId;

    //expound the receiverId, for faster access of sipp indexes
    if ( hndl->receiverType == CIF_TYPE )
    {
        hndl->cifId  = userSpec->receiverId;
        hndl->sippId = 0;
    }
    else
    {
        hndl->sippId = userSpec->receiverId - SIPP_DEVICE0;
        hndl->cifId  = 0;
    }

    //copy the entire user/camera parameters list in the handler context, instead of only storing the pointer
    memcpy(&hndl->usrSpec, userSpec, sizeof(CamUserSpec));
    memcpy(&hndl->camSpec, camSpec, sizeof(GenericCamSpec));

    hndl->runningFrame                = NULL;
    hndl->nextFrame                   = NULL;
    hndl->pI2cHandle                  = pI2cHandle;

    if ( cbList != NULL )
    {
        if ( cbList->isrCbfList != NULL )
        {
            hndl->isrFPtr.getBlock     = cbList->isrCbfList->getBlock;
            hndl->isrFPtr.getFrame     = cbList->isrCbfList->getFrame;
            hndl->isrFPtr.notification = cbList->isrCbfList->notification;
        }
        else
        {
            hndl->isrFPtr.getBlock         = NULL;
            hndl->isrFPtr.getFrame         = NULL;
            hndl->isrFPtr.notification     = NULL;
        }

        if ( cbList->sensorCbfList != NULL )
        {
            hndl->sensorFPtr.sensorPowerUp    = cbList->sensorCbfList->sensorPowerUp;
            hndl->sensorFPtr.sensorStandby    = cbList->sensorCbfList->sensorStandby;
            hndl->sensorFPtr.sensorWakeup     = cbList->sensorCbfList->sensorWakeup;
            hndl->sensorFPtr.sensorPowerDown  = cbList->sensorCbfList->sensorPowerDown;
        }
        else
        {
            hndl->sensorFPtr.sensorPowerUp    = NULL;
            hndl->sensorFPtr.sensorStandby    = NULL;
            hndl->sensorFPtr.sensorWakeup     = NULL;
            hndl->sensorFPtr.sensorPowerDown  = NULL;
        }
    }

    //chek the match between the user parameters and the static configuration (initial values of sippRegsArray / cifRegsArray )
    if (hndl->receiverType == SIPP_TYPE)
    {
        isrManagement   = sippRegsArray[hndl->sippId].int_management_line_mask;  //check the static configuration
        if ( isrManagement == 0 )
            isrManagement   = sippRegsArray[hndl->sippId].int_management_frame_mask;
        isrNotification = sippRegsArray[hndl->sippId].int_notification_line_mask;
        if ( isrNotification == 0 )
            isrNotification = sippRegsArray[hndl->sippId].int_notification_frame_mask;
    }
    else
    {
        isrManagement   = cifRegsArray[hndl->cifId].int_management_mask;   //check the static configuration
        isrNotification = cifRegsArray[hndl->cifId].int_notification_mask; //
    }

    //hndl->interruptsPriority = CAMGENERIC_INTERRUPT_LEVEL;  //by default, at init, the interrupts level is the one statically configured in the CamGeneric header files
    hndl->interruptsPriority = 14;  //by default, at init, the interrupts level is the one statically configured in the CamGeneric header files

    //ISR callbacks validation
    //allocation callbacks
    if ( isrManagement != 0 )  //check the default management mask: ISR management to be performed by CamGeneric?
    {
        //one and only one buffer allocation function has to be present
        //        if ( (hndl->isrFPtr.getFrame == NULL) && ( hndl->isrFPtr.getBlock == NULL ) )
        //           return (CAM_ERR_PARAMETERS_LIST_INCOMPLETE);

        if ( (hndl->isrFPtr.getFrame != NULL) && ( hndl->isrFPtr.getBlock != NULL ) )
            return (CAM_ERR_CONFLICTING_ALLOC_FUNCTIONS);
    }
    else
        //no buffer allocation function has to be present
        if ( (hndl->isrFPtr.getFrame != NULL) || ( hndl->isrFPtr.getBlock != NULL ) )
            return (CAM_ERR_CONFLICTING_ALLOC_FUNCTIONS);

    //notification callbacks; if notification requested, the notification callback must be present
    if (( isrNotification != 0 ) && ( hndl->isrFPtr.notification == NULL ))
        return (CAM_ERR_PARAMETERS_LIST_INCOMPLETE);

    //set the IRQ id for ICB block
    //automatically reroute the ISRs if the code is running on LOS; in order to force LRT handling in LOS case, the CamSetupInterrupts must be called
    if ( PROCESS_LEON_OS == swcWhoAmI() )
    {
        //reroute the ISRs
        switch (hndl->receiverId)
        {
        case (CIF_DEVICE0):
                                       cifRegsArray[CIF_DEVICE0].int_id = ROUTED_IRQ_CIF0;
        break;
        case (CIF_DEVICE1):
                                       cifRegsArray[CIF_DEVICE1].int_id = ROUTED_IRQ_CIF1;
        break;
        case (SIPP_DEVICE0):
        case (SIPP_DEVICE1):
        case (SIPP_DEVICE2):
        case (SIPP_DEVICE3):
        sippRegsArray[hndl->sippId].line_int_id  = ROUTED_IRQ_SIPP_LINE_ALL;
        sippRegsArray[hndl->sippId].frame_int_id = ROUTED_IRQ_SIPP_FRAME_ALL;
        break;
        }
    }
    else
    {
        //use the default ISR id's
        switch (hndl->receiverId)
        {
        case (CIF_DEVICE0):
        case (CIF_DEVICE1):
        cifRegsArray[hndl->cifId].int_id = cifRegsArray[hndl->cifId].int_id_default;
        break;
        case (SIPP_DEVICE0):
        case (SIPP_DEVICE1):
        case (SIPP_DEVICE2):
        case (SIPP_DEVICE3):
        sippRegsArray[hndl->sippId].line_int_id  = sippRegsArray[hndl->sippId].line_int_id_default;
        sippRegsArray[hndl->sippId].frame_int_id = sippRegsArray[hndl->sippId].frame_int_id_default;
        break;
        }
    }

    camHndlArray[hndl->receiverId] = hndl;

    //after the initialization step, the camera does not become active but is left in a hot standby state;
    // thus, custom parameterization of the camera component may be performed right after CamInit, before CamStart
    hndl->status     = CAM_STATUS_HOT_STANDBY;

    if ( hndl->camSpec.mipiCfg->nbOflanes > 0 ) //MIPI connection
    {
        //TODO: fully check the mipi - receiver connection validity inside MSS (partially checked inside DrvMssConnectMipiToDevice)
        if ( -1 == DrvMssConnectMipiToDevice(hndl->usrSpec.mipiControllerNb, (hndl->receiverType == SIPP_TYPE ? DRV_MSS_SIPP_RX : DRV_MSS_CIF) ) )  //conversion to MSS type
            return (CAM_ERR_FEATURE_NOT_AVAILABLE_FOR_CURRENT_CONFIGURATION);

        mipiRetValue = InitMIPI( &hndl->camSpec, hndl->usrSpec.mipiControllerNb );

        if ( mipiRetValue != DRV_MIPI_CFG_DONE )
        {
            DPRINTF("\n MIPI ERROR (eDrvMipiInitFuncRet) : %ld\n", (u32)mipiRetValue);
            if ( mipiRetValue != DRV_MIPI_CFG_PLL_NOT_LOCKED )
                retValue = CAM_ERR_MIPI_CONFIGURATION_ERROR;
            else
                retValue = CAM_ERR_MIPI_PLL_INITIALISATION_ERROR;

            return retValue;
        }

    }
    else
    {
        if ( hndl->receiverId == CIF_DEVICE1) //only CIF1 can accept sensors connected on parallel bus (through GPIOs)
            DrvMssConnectCif1ToGpio();
        else
            return (CAM_ERR_FEATURE_NOT_AVAILABLE_FOR_CURRENT_CONFIGURATION);
    }

    if (hndl->receiverType == CIF_TYPE)
    {
        if ( hndl->camSpec.mipiCfg->nbOflanes == 0 ) //parallel connection
            DrvCifSetMclkFrequency( (hndl->receiverId == CIF_DEVICE0 ? CAMERA_1 : CAMERA_2), (hndl->camSpec.idealRefFreq * 1000)); //conversion to CIF type

        DrvCifInit( (hndl->receiverId == CIF_DEVICE0 ? CAMERA_1 : CAMERA_2) );
        CifCfg(hndl, cifRegsArray[hndl->cifId].CIF_BASE_ADR, &hndl->camSpec, hndl->usrSpec.windowRowStart, hndl->usrSpec.windowColumnStart, hndl->usrSpec.windowWidth, hndl->usrSpec.windowHeight);
    }
    else
    {
        DrvSippReset( 1<< (hndl->sippId + SIPP_MIPI_RX0_ID) );
        //configure but don't start the SIPP
        SippCfg(hndl, hndl->sippId, &hndl->camSpec, hndl->usrSpec.windowRowStart, hndl->usrSpec.windowColumnStart, hndl->usrSpec.windowWidth, hndl->usrSpec.windowHeight);
    }

    //perform partial sensor configuration here, up to the last register (not including)
    if ( hndl->sensorFPtr.sensorPowerUp != NULL )
        retValue = hndl->sensorFPtr.sensorPowerUp(hndl->pI2cHandle, &hndl->camSpec, &hndl->usrSpec);
    else
        retValue = defaultSensorPowerUp(hndl);
    // TODO(zhoury): Now we force to set retValue = 0 for nextchip ISP
    retValue = 0;

    return retValue;
}

extern camErrorType CamStart(GenericCameraHandle *hndl)
{
    camErrorType retValue = CAM_SUCCESS;
    SippHwRegsAndMasks  *sippRegistersSet = NULL;
    CifHwRegsAndMasks   *cifRegistersSet = NULL;
    u32 cifRegisterContent, regsBaseAddr;
    u32 reRouteIsr;

    if (hndl == NULL)
        return (CAM_ERR_PARAMETERS_LIST_INCOMPLETE);

    if ( hndl->receiverType == CIF_TYPE )
    {
        cifRegistersSet  = &cifRegsArray[hndl->cifId];

        regsBaseAddr = cifRegistersSet->CIF_BASE_ADR;

        reRouteIsr = ((cifRegistersSet->int_id >= IRQ_DYNAMIC_0) ? 1 : 0);
    }
    else
    {
        sippRegistersSet = &sippRegsArray[hndl->sippId];

        regsBaseAddr = sippRegistersSet->SIPP_BASE_ADR;

        if (( sippRegistersSet->line_int_id >= IRQ_DYNAMIC_0 ) || ( sippRegistersSet->frame_int_id >= IRQ_DYNAMIC_0 ))
            reRouteIsr = 1;
        else
            reRouteIsr = 0;
    }

    DPRINTF("Reroute ISR to LOS: %ld\n", reRouteIsr);

    hndl->status        = CAM_STATUS_ACTIVE;
    hndl->frameCount    = 0;
    hndl->receivedFrame = NULL;

    //request a data buffer address
    if (hndl->isrFPtr.getBlock != NULL)
    {
        hndl->runningFrame = hndl->isrFPtr.getBlock(0);
        assert(hndl->runningFrame     != NULL);
        assert(hndl->runningFrame->p1 != NULL);                  //at least one pointer (p1) out of three (p1,p2,p3) has to be present; TODO assert all 3, based on the frame format?
    }
    else if (hndl->isrFPtr.getFrame != NULL)
    {
        hndl->runningFrame = hndl->isrFPtr.getFrame();
        assert(hndl->runningFrame     != NULL);         //TODO: remove redundancy
        assert(hndl->runningFrame->p1 != NULL);              //at least one pointer (p1) out of three (p1,p2,p3) has to be present; TODO assert all 3, based on the frame format?
    }
    else
    {
        hndl->runningFrame = NULL;  //just in case it's not already NULL
    }

    //conditionally start the ISRs and the receiver DMA(s)
    if (hndl->receiverType == SIPP_TYPE)
    {
        if ((sippRegistersSet->int_management_line_mask != 0) || (sippRegistersSet->int_notification_line_mask != 0))
        {
            // clear all pending interrupts at source/in ICB
            SET_REG_WORD(SIPP_INT1_CLEAR_ADR, sippRegistersSet->int_clear_line_mask);
            GET_REG_WORD_VAL(SIPP_INT1_STATUS_ADR);   //dummy read in order to guarantee the transaction order, in case of LOS
            DrvIcbIrqClear(sippRegistersSet->line_int_id);

            //enable/clear interrupts rerouting to LOS
            DrvIcbDynamicIrqConfig(LRT_TO_LOS, IRQ_SIPP_1, sippRegistersSet->line_int_id, reRouteIsr);

            //enable LEVEL interrupt generation/routing in ICB
            DrvIcbSetupIrq(sippRegistersSet->line_int_id, hndl->interruptsPriority, POS_LEVEL_INT, (irq_handler)&SippCamHandler);

            //enable specific interrupts generation in the receiver
            SET_REG_WORD(SIPP_INT1_ENABLE_ADR, GET_REG_WORD_VAL(SIPP_INT1_ENABLE_ADR) | sippRegistersSet->int_management_line_mask);
        }

        if ((sippRegistersSet->int_management_frame_mask != 0) || (sippRegistersSet->int_notification_frame_mask != 0))
        {
            // clear all pending interrupts at source/in ICB
            SET_REG_WORD(SIPP_INT2_CLEAR_ADR, sippRegistersSet->int_clear_frame_mask);
            GET_REG_WORD_VAL(SIPP_INT2_STATUS_ADR);   //dummy read in order to guarantee the transaction order, in case of LOS
            DrvIcbIrqClear(sippRegistersSet->frame_int_id);

            //enable/clear interrupts rerouting to LOS
            DrvIcbDynamicIrqConfig(LRT_TO_LOS, IRQ_SIPP_2, sippRegistersSet->frame_int_id, reRouteIsr);

            //enable LEVEL interrupt generation/routing in ICB
            DrvIcbSetupIrq(sippRegistersSet->frame_int_id, hndl->interruptsPriority, POS_LEVEL_INT, (irq_handler)&SippCamHandler);

            //enable specific interrupts generation in the receiver
            SET_REG_WORD(SIPP_INT2_ENABLE_ADR, GET_REG_WORD_VAL(SIPP_INT2_ENABLE_ADR) | sippRegistersSet->int_management_frame_mask);
        }

        //in case buffer management by CamGeneric was requested, update the buffer address and start the already configured SIPP receiver;
        //otherwise, the application has to perform these steps before calling CamStart (CamGeneric will only configure additional IRQ infrastructure and the sensor)
        if (hndl->runningFrame    != NULL)
        // (dummy SIPP stop +) buffer address update + SIPP start + next buffer setup;  //GASTEMP needed ????
          DrvSippEngineRestart(hndl->runningFrame->p1, sippRegistersSet->SIPP_BASE_ADR, hndl->sippId );
    }
    else //CIF type
        if ( (hndl->runningFrame != NULL) || (hndl->isrFPtr.notification != NULL) )
            //if either management or notification has to be performed on events by CamGeneric, then start the ISRs
        {
            // clear all pending interrupts at source
            SET_REG_WORD(cifRegistersSet->INT_CLEAR_ADR, cifRegistersSet->int_clear_mask);

            DrvIcbDynamicIrqConfig(LRT_TO_LOS, ((hndl->receiverId==CIF_DEVICE0)? IRQ_CIF0 : IRQ_CIF1), cifRegistersSet->int_id, reRouteIsr);

            //enable EDGE interrupt generation in ICB
            DrvIcbSetupIrq(cifRegistersSet->int_id, hndl->interruptsPriority, POS_EDGE_INT, (irq_handler)&CifCamHandler);

            //enable specific interrupts generation in the receiver
            cifRegisterContent = cifRegistersSet->int_management_mask;
            cifRegisterContent |= cifRegistersSet->int_notification_mask;
            SET_REG_WORD(cifRegistersSet->INT_ENABLE_ADR, cifRegisterContent);

            //in case buffer management by CamGeneric was requested, update the buffer(s) address(es) and start the already configured CIF receiver;
            //otherwise, the application has to perform these steps before calling CamStart (CamGeneric will only configure additional IRQ infrastructure and the sensor)
            if (hndl->runningFrame != NULL)
            {
                //update the DMA addresses and start; assumption: all pointers already validated
                DrvCifDma0StartPP(regsBaseAddr, (u32)hndl->runningFrame->p1, (u32)hndl->runningFrame->p1);

                if (hndl->runningFrame->p2 != NULL)
                {
                    DrvCifDma1StartPP(regsBaseAddr, (u32)hndl->runningFrame->p2, (u32)hndl->runningFrame->p2);
                }

                if (hndl->runningFrame->p3 != NULL)
                {
                    DrvCifDma2StartPP(regsBaseAddr, (u32)hndl->runningFrame->p3, (u32)hndl->runningFrame->p3);
                }

                if (cifRegsArray[hndl->cifId].MIPI_sync != NO_RESYNC)  //for now, only statically configurable in this file; TODO either provide an API or remove this feature, based on feedback
                    cifResyncSetup(hndl->camSpec.mipiCfg->nbOflanes, (u32)hndl->usrSpec.mipiControllerNb, reRouteIsr);

            }
        }

    // the start of the sensor is treated as a normal wake up, because, at the end of the initialization part in CamInit/CamStandby, the sensor is left in standby
    if ( hndl->sensorFPtr.sensorWakeup != NULL )
        retValue = hndl->sensorFPtr.sensorWakeup(hndl->pI2cHandle, &hndl->camSpec, &hndl->usrSpec);
    else if ( hndl->pI2cHandle != NULL )
        retValue = defaultSensorWakeup(hndl);
    else
        retValue = CAM_ERR_MISSING_CALLBACK_OR_I2C_HANDLE;

    // TODO(zhoury): Now we force to set retValue = 0 for nextchip ISP
    retValue = 0;

    return retValue;
}

static void CifCamStop(GenericCameraHandle *hndl)
{
    u32 cifId;

    cifId = hndl->cifId;

    SET_REG_WORD(cifRegsArray[cifId].CIF_BASE_ADR + CIF_DMA0_CFG_OFFSET, (GET_REG_WORD_VAL(cifRegsArray[cifId].CIF_BASE_ADR + CIF_DMA0_CFG_OFFSET) & (~D_CIF_DMA_ENABLE) ));
    SET_REG_WORD(cifRegsArray[cifId].CIF_BASE_ADR + CIF_DMA1_CFG_OFFSET, (GET_REG_WORD_VAL(cifRegsArray[cifId].CIF_BASE_ADR + CIF_DMA1_CFG_OFFSET) & (~D_CIF_DMA_ENABLE) ));
    SET_REG_WORD(cifRegsArray[cifId].CIF_BASE_ADR + CIF_DMA2_CFG_OFFSET, (GET_REG_WORD_VAL(cifRegsArray[cifId].CIF_BASE_ADR + CIF_DMA2_CFG_OFFSET) & (~D_CIF_DMA_ENABLE) ));

    SET_REG_WORD(cifRegsArray[cifId].CIF_BASE_ADR + CIF_FIFO_FLUSH_OFFSET, 1); //flush the DMA FIFO

    while(((GET_REG_WORD_VAL(cifRegsArray[cifId].CIF_BASE_ADR + CIF_DMA0_CFG_OFFSET)) & D_CIF_DMA_ACTIVITY_MASK));
    while(((GET_REG_WORD_VAL(cifRegsArray[cifId].CIF_BASE_ADR + CIF_DMA1_CFG_OFFSET)) & D_CIF_DMA_ACTIVITY_MASK));
    while(((GET_REG_WORD_VAL(cifRegsArray[cifId].CIF_BASE_ADR + CIF_DMA2_CFG_OFFSET)) & D_CIF_DMA_ACTIVITY_MASK));

    SET_REG_WORD(cifRegsArray[cifId].CIF_BASE_ADR + CIF_CONTROL_OFFSET,  0);

    DrvCifReset( cifId == 0 ? CAMERA_1 : CAMERA_2);

    SET_REG_WORD(cifRegsArray[cifId].INT_CLEAR_ADR, cifRegsArray[cifId].int_clear_mask);

    if (( cifRegsArray[cifId].int_management_mask != 0 ) ||
                    ( cifRegsArray[cifId].int_notification_mask != 0 ) )
    {
        DrvIcbDisableIrq(cifRegsArray[cifId].int_id);
        DrvIcbDynamicIrqConfig(LRT_TO_LOS, cifRegsArray[cifId].int_id_default, cifRegsArray[cifId].int_id, 0); //disable routing to LOS (if active)

        cifResyncStop(hndl->camSpec.mipiCfg->nbOflanes, hndl->usrSpec.mipiControllerNb);
    }
}

static void SippCamStop(u32 sippId)
{
    u8 isrHandlingOngoing = 0, sippUsersPresent = 1;

    DrvSippDisableRxFilter( sippId );
    DrvSippReset( 1<<(sippId + SIPP_MIPI_RX0_ID) ); //also flushing here

    SET_REG_WORD(SIPP_INT1_CLEAR_ADR, sippRegsArray[sippId].int_clear_line_mask);
    SET_REG_WORD(SIPP_INT2_CLEAR_ADR, sippRegsArray[sippId].int_clear_frame_mask);

    //stop the line ISR generation/rerouting only if no more SIPP receivers are active on line
    if (( sippRegsArray[sippId].int_management_line_mask != 0 ) ||
                    ( sippRegsArray[sippId].int_notification_line_mask != 0 ))
    {
        isrHandlingOngoing = 1;
        if (( sippRegsArray[0].int_management_line_mask == 0 ) && ( sippRegsArray[0].int_notification_line_mask == 0 ) &&
                        ( sippRegsArray[1].int_management_line_mask == 0 ) && ( sippRegsArray[1].int_notification_line_mask == 0 ) &&
                        ( sippRegsArray[2].int_management_line_mask == 0 ) && ( sippRegsArray[2].int_notification_line_mask == 0 ) &&
                        ( sippRegsArray[3].int_management_line_mask == 0 ) && ( sippRegsArray[3].int_notification_line_mask == 0 ))
        {

            sippUsersPresent = 0; //no more SIPP receivers in line mode are present
        }

        if ((isrHandlingOngoing == 1) && (sippUsersPresent == 0))
        {
            DrvIcbDisableIrq(sippRegsArray[sippId].line_int_id);
            DrvIcbDynamicIrqConfig(LRT_TO_LOS, IRQ_SIPP_1, sippRegsArray[sippId].line_int_id, 0);
        }
    }

    isrHandlingOngoing = 0;
    sippUsersPresent = 1;

    //stop the frame ISR generation/rerouting only if no more SIPP receivers are active on frame
    if (( sippRegsArray[sippId].int_management_frame_mask != 0 ) ||
                    ( sippRegsArray[sippId].int_notification_frame_mask != 0 ))
    {
        isrHandlingOngoing = 1;
        if (( sippRegsArray[0].int_management_frame_mask == 0 ) && ( sippRegsArray[0].int_notification_frame_mask == 0 ) &&
                        ( sippRegsArray[1].int_management_frame_mask == 0 ) && ( sippRegsArray[1].int_notification_frame_mask == 0 ) &&
                        ( sippRegsArray[2].int_management_frame_mask == 0 ) && ( sippRegsArray[2].int_notification_frame_mask == 0 ) &&
                        ( sippRegsArray[3].int_management_frame_mask == 0 ) && ( sippRegsArray[3].int_notification_frame_mask == 0 ))
        {
            sippUsersPresent = 0; //SIPP receivers in line mode are present
        }

        if ((isrHandlingOngoing == 1) && (sippUsersPresent == 0))
        {
            DrvIcbDisableIrq(sippRegsArray[sippId].frame_int_id);
            DrvIcbDynamicIrqConfig(LRT_TO_LOS, IRQ_SIPP_2, sippRegsArray[sippId].frame_int_id, 0);
        }
    }
}

extern camErrorType CamStandby(GenericCameraHandle *hndl, camStatus_type standbyType)
{
    camErrorType retValue = CAM_SUCCESS;

    if (hndl == NULL)
        return (CAM_ERR_PARAMETERS_LIST_INCOMPLETE);

    if ( standbyType != CAM_STATUS_ACTIVE )
        hndl->status = standbyType;
    else
        return (CAM_ERR_STATUS_NOT_APPROPRIATE);

    if ( hndl->status == CAM_STATUS_COLD_STANDBY )
    {
        if ( hndl->sensorFPtr.sensorPowerDown != NULL )
            retValue = hndl->sensorFPtr.sensorPowerDown(hndl->pI2cHandle, &hndl->camSpec, &hndl->usrSpec);
        else if ( hndl->pI2cHandle != NULL )
            retValue = defaultSensorPowerDown(hndl);
        else
            retValue = CAM_ERR_MISSING_CALLBACK_OR_I2C_HANDLE;
    }
    else
    {
        if ( hndl->sensorFPtr.sensorStandby != NULL )
            retValue = hndl->sensorFPtr.sensorStandby(hndl->pI2cHandle, &hndl->camSpec, &hndl->usrSpec);
        else if ( hndl->pI2cHandle != NULL )
            retValue = defaultSensorStandby(hndl);
        else
            retValue = CAM_ERR_MISSING_CALLBACK_OR_I2C_HANDLE;
    }
    // TODO(zhoury): Now we force to set retValue = 0 for nextchip ISP
    retValue = 0;
    if (hndl->receiverType == CIF_TYPE )
    {
        CifCamStop(hndl);
        //in advance CIF reconfiguration (without enabling), in order to save time at wake up
        CifCfg(hndl, cifRegsArray[hndl->cifId].CIF_BASE_ADR, &hndl->camSpec, hndl->usrSpec.windowRowStart, hndl->usrSpec.windowColumnStart, hndl->usrSpec.windowWidth, hndl->usrSpec.windowHeight);
    }
    else
    {
        SippCamStop(hndl->sippId);
        //in advance SIPP reconfiguration (without enabling), in order to save time at wake up
        SippCfg(hndl, hndl->sippId, &hndl->camSpec, hndl->usrSpec.windowRowStart, hndl->usrSpec.windowColumnStart, hndl->usrSpec.windowWidth, hndl->usrSpec.windowHeight);
    }

    return retValue;
}

extern camErrorType CamWakeup(GenericCameraHandle *hndl)
{
    camErrorType retValue = CAM_SUCCESS;
    eDrvMipiInitFuncRet mipiRetValue;

    if (hndl == NULL)
        return (CAM_ERR_PARAMETERS_LIST_INCOMPLETE);

    if ( hndl->status == CAM_STATUS_ACTIVE )
        return (CAM_ERR_STATUS_NOT_APPROPRIATE);

    // full my2 components reset/reconfiguration needed (but shifted most of them to CamStandby) because of the MIPI PHY which sometimes to get stuck on sensor restart

    if ( hndl->camSpec.mipiCfg->nbOflanes > 0 ) //MIPI connection
    {
        mipiRetValue = InitMIPI( &hndl->camSpec, hndl->usrSpec.mipiControllerNb );

        if ( mipiRetValue != DRV_MIPI_CFG_DONE )
        {
            DPRINTF("\n MIPI ERROR (eDrvMipiInitFuncRet) : %ld\n", (u32)mipiRetValue);
            if ( mipiRetValue != DRV_MIPI_CFG_PLL_NOT_LOCKED )
                retValue = CAM_ERR_MIPI_CONFIGURATION_ERROR;
            else
                retValue = CAM_ERR_MIPI_PLL_INITIALISATION_ERROR;

            return retValue;
        }
    }

    if ( hndl->status == CAM_STATUS_COLD_STANDBY )
    {
        //fully reconfigure the sensor
        if ( hndl->sensorFPtr.sensorPowerUp != NULL )
            retValue = hndl->sensorFPtr.sensorPowerUp(hndl->pI2cHandle, &hndl->camSpec, &hndl->usrSpec);
        else if ( hndl->pI2cHandle != NULL )
            retValue = defaultSensorPowerUp(hndl);
        else
            retValue = CAM_ERR_MISSING_CALLBACK_OR_I2C_HANDLE;
    }

    if ( retValue != CAM_SUCCESS )
        return retValue;

    retValue = CamStart(hndl);   //restart ISR + SIPP/CIF receivers + the sensor

    if ( retValue == CAM_SUCCESS )
        hndl->status = CAM_STATUS_ACTIVE;

    return retValue;
}

extern camErrorType CamStop(GenericCameraHandle *hndl)
{
    camErrorType retValue = CAM_SUCCESS;

    if (hndl == NULL)
        return (CAM_ERR_PARAMETERS_LIST_INCOMPLETE);

    if ( hndl->sensorFPtr.sensorPowerDown != NULL )
        retValue = hndl->sensorFPtr.sensorPowerDown(hndl->pI2cHandle, &hndl->camSpec, &hndl->usrSpec);
    else if ( hndl->pI2cHandle != NULL )
        retValue = defaultSensorPowerDown(hndl);
    else
        retValue = CAM_ERR_MISSING_CALLBACK_OR_I2C_HANDLE;

    retValue = 0;
    if ( hndl->receiverType == SIPP_TYPE)
        SippCamStop(hndl->sippId);
    else
        CifCamStop(hndl);

    memset(hndl, 0, sizeof(GenericCameraHandle));

    return (retValue);
}

extern unsigned int CamGetFrameCounter (GenericCameraHandle *hndl)
{
    //this function is useful for CIF receivers only and will return the current
    u32 retValue = 0;

    if (hndl != NULL)
        retValue = hndl->frameCount;

    return retValue;
}
