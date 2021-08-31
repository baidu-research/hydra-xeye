///
/// @file
/// @copyright All code copyright Movidius Ltd 2015, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Camera wrapper.
///
/// This implements additional CIF re-synchronization functionality using MIPI interrupts
///
///

// 1: Includes
// ----------------------------------------------------------------------------
// Drivers includes

#include "assert.h"
#include "string.h"

#include "registersMyriad.h"
#include "DrvRegUtils.h"
#include "DrvIcb.h"
#include "DrvMipi.h"
#include "DrvCif.h"
#include "DrvCpr.h"


#include "CamGenericApi.h"
#include "CamGenericPrivateDefines_ma2x5x.h"

extern CifHwRegsAndMasks cifRegsArray[];
//-------------------------------------------
//#define _LOCAL_DEBUG_

#ifdef _LOCAL_DEBUG_
   #include "stdio.h"
   #define DPRINTF(...) printf(__VA_ARGS__)
#else
   #define DPRINTF(...)
#endif


#define D_MIPI_CSI_FRAME_END            0x01

static void stopMipiIrq(u32 mipiCtrlNo);

//---------------------------------------------------------------
static void mipiIrqHandler(u32 unused)
{
    UNUSED(unused); // hush the compiler warning.

    u32 intStatus;

    // Get/clear the status
    intStatus = GET_REG_WORD_VAL(MIPI_IRQ_STATUS_ADDR);
    SET_REG_WORD(MIPI_IRQ_CLEAR_ADDR, intStatus);

    if (intStatus & (D_MIPI_IRQ_STAT_HS_RX_EVENT_0 | D_MIPI_IRQ_STAT_HS_RX_EVENT_1))
    {
        // Disable and re-enable CIF1 windowing
        DrvCifWindowReset(CIF1_BASE_ADR);

        if (cifRegsArray[CIF_DEVICE0].MIPI_sync == RESYNC_ONCE)
        {
            DPRINTF("\nMIPI-CIF1 sync stopped\n");
            if (intStatus & D_MIPI_IRQ_STAT_HS_RX_EVENT_0)
                stopMipiIrq(DRV_MIPI_C0);
            else
                stopMipiIrq(DRV_MIPI_C1);
        }
    }

    if (intStatus & (D_MIPI_IRQ_STAT_HS_RX_EVENT_3 | D_MIPI_IRQ_STAT_HS_RX_EVENT_5))
    {
        // Disable and re-enable CIF2 windowing
        DrvCifWindowReset(CIF2_BASE_ADR);

        if (cifRegsArray[CIF_DEVICE1].MIPI_sync == RESYNC_ONCE)
        {
            DPRINTF("\nMIPI-CIF2 sync stopped\n");
            if (intStatus & D_MIPI_IRQ_STAT_HS_RX_EVENT_3)
                stopMipiIrq(DRV_MIPI_C3);
            else
                stopMipiIrq(DRV_MIPI_C5);
        }
    }
}

static u32 getMipiBase(u32 mipiCtrlNo)
{
    switch (mipiCtrlNo)
    {
        case DRV_MIPI_C0:
            return (APB_MIPI0_HS_BASE_ADDR);
            break;
        case DRV_MIPI_C1:
            return (APB_MIPI1_HS_BASE_ADDR);
            break;
        case DRV_MIPI_C3:
            return (APB_MIPI3_HS_BASE_ADDR);
            break;
        case DRV_MIPI_C5:
            return (APB_MIPI5_HS_BASE_ADDR);
            break;
        default :
            assert(0);
    }
}

static void setupMipiIrq(u32 mipiCtrlNo, u32 reRouteIsr)
{
    u32 mipiBaseReg;

    SET_REG_WORD(MIPI_IRQ_CLEAR_ADDR, (D_MIPI_IRQ_STAT_HS_RX_EVENT_0 << mipiCtrlNo));

    mipiBaseReg = getMipiBase(mipiCtrlNo);

    //setup Mipi to generate IRQ on frame end packet
    SET_REG_WORD(mipiBaseReg + MIPI_RX_HS_PH_FILTER0_OFFSET, (1 << D_MIPI_CSI_FRAME_END) );
    SET_REG_WORD(mipiBaseReg + MIPI_RX_HS_PH_FILTER1_OFFSET, 0);
    SET_REG_WORD(MIPI_IRQ_ENABLE_ADDR,  GET_REG_WORD_VAL(MIPI_IRQ_ENABLE_ADDR) | (D_MIPI_IRQ_STAT_HS_RX_EVENT_0 << mipiCtrlNo));

    DrvIcbDynamicIrqConfig(LRT_TO_LOS, IRQ_MIPI, ROUTED_IRQ_MIPI, reRouteIsr);
    DrvIcbSetupIrq(reRouteIsr ? ROUTED_IRQ_MIPI : IRQ_MIPI, MIPI_INTERRUPT_LEVEL, POS_EDGE_INT, (irq_handler)&mipiIrqHandler);
}

static void stopMipiIrq(u32 mipiCtrlNo)
{
    u32 mipiBaseReg;
    u32 reRouteIsr;
    camRxId_type devId = CIF_DEVICE0;

    mipiBaseReg = getMipiBase(mipiCtrlNo);

    //stop Mipi to generate IRQ on frame end packet
    SET_REG_WORD(mipiBaseReg + MIPI_RX_HS_PH_FILTER0_OFFSET, 0);
    SET_REG_WORD(mipiBaseReg + MIPI_RX_HS_PH_FILTER1_OFFSET, 0);
    SET_REG_WORD(MIPI_IRQ_ENABLE_ADDR,  GET_REG_WORD_VAL(MIPI_IRQ_ENABLE_ADDR) & ~(D_MIPI_IRQ_STAT_HS_RX_EVENT_0 << mipiCtrlNo));

    //disable ISR generation in ICB only if there is no other MIPI generating ISRs
    if ((GET_REG_WORD_VAL(MIPI_IRQ_ENABLE_ADDR) == 0 ))
    {
        /* getMipiBase() above would assert() if mipiCtrlNo is invalid */
        if ((mipiCtrlNo == DRV_MIPI_C3) || (mipiCtrlNo == DRV_MIPI_C5))
            devId = CIF_DEVICE1;

        reRouteIsr = (cifRegsArray[devId].int_id >= IRQ_DYNAMIC_0) ? 1 : 0;

        DrvIcbDisableIrq(reRouteIsr ? ROUTED_IRQ_MIPI : IRQ_MIPI);
        DrvIcbDynamicIrqConfig(LRT_TO_LOS, IRQ_MIPI, ROUTED_IRQ_MIPI, 0);
    }
}

void cifResyncSetup(u32 nbOflanes, u32 mipiCtrlNo, u32 reRouteIsr)
{
    if ( nbOflanes > 0 )
        setupMipiIrq(mipiCtrlNo, reRouteIsr);
    else
    {
        //TODO: implement GPIO VSYNC edge workaround for parallel sensors (equivalent for MIPI workaround, see 22697)
    }
}

void cifResyncStop(u32 nbOflanes, u32 mipiCtrlNo)
{
    if ( nbOflanes > 0 )
        stopMipiIrq(mipiCtrlNo);
    else
    {
        //TODO: stop the equivalent GPIO ISR generation for parallel sensors
    }
}
