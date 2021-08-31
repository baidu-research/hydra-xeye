///
/// @file
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved
///            For License Warranty see: common/license.txt
///
/// @brief     Definitions and types needed by the Math operations library
///
/// This file contains all the private definitions of constants, typedefs,
/// structures and enums for the generic camera component

#ifndef _CAM_GENERIC_PRIVATE_DEFINES_H_
#define _CAM_GENERIC_PRIVATE_DEFINES_H_


// 1: Defines
// ----------------------------------------------------------------------------
//level for camera interrupts
#define CAMGENERIC_INTERRUPT_LEVEL  3

//dynamic IRQ's used
#define ROUTED_IRQ_CIF0            IRQ_DYNAMIC_0 //all of the CIF0 isr are presented on the same line
#define ROUTED_IRQ_CIF1            IRQ_DYNAMIC_1 //all of the CIF1 isr are presented on the same line
#define ROUTED_IRQ_SIPP_LINE_ALL   IRQ_DYNAMIC_2 //all 3 SIPP line isr are presented on the same line
#define ROUTED_IRQ_SIPP_FRAME_ALL  IRQ_DYNAMIC_4 //all 3 SIPP frame isr are presented on the same line

//2: Internal function prototypes
void cifResyncSetup(u32 nbOflanes, u32 mipiCtrlNo, u32 reRouteIsr);
void cifResyncStop(u32 nbOflanes, u32 mipiCtrlNo);


#endif //_CAM_GENERIC_PRIVATE_DEFINES_H_
