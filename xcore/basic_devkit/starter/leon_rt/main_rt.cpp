///
/// @file      main_rt.cpp
/// @copyright All code copyright Movidius Ltd 2017, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Main Rt application.
///            Platform(s) supported : ma2x5x
///

#include <stdlib.h>
#include <stdio.h>
#include <Flic.h>
#include <FlicRmt.h>
#include <MemAllocator.h>
#include <Pool.h>
#include <DrvLeon.h>
#include "mv_types.h"
#include "OsDrvMipi.h"
#include <ImgFrame.h>
#include <CmdGenMsg.h>
#include <GrpIspColorVdoZsl2L_RT.h>
#include <rtems/cpuuse.h>

extern GrpIspColorVdoZsl2L_OS  grpIspColorVdoZsl2L_OS;
GrpIspColorVdoZsl2L_RT         grpIspColorVdoZsl2L_RT      __attribute__((section(".cmx.cdmaDescriptors")));
Pipeline   __attribute__((section(".cmx.cdmaDescriptors")))   p(48);
uint32_t __attribute__((section(".cmx_direct.data"))) showCpuReport = 0;

//####################################################################
extern "C" void * POSIX_Init(void *args)
{
   UNUSED(args);
   pthread_setname_np(RTEMS_SELF, "main");
   // init mipi Rx/Tx driver, Register all mipi rx an tx controller.
   uint32_t mipi_major;
   int sc;
   struct OsDrvMipiInitDriverCfg driver_config = {
     .irq_priority = 14,
     .loopback_mode = 0,
   };
   sc = rtems_io_register_driver(0, &mipi_drv_tbl, &mipi_major);
   assert(sc == RTEMS_SUCCESSFUL);
   sc = rtems_io_initialize(mipi_major, 0, &driver_config);
   assert(sc == RTEMS_SUCCESSFUL);

   FlicRmt::Init();
   RgnAlloc  .Create(RgnBuff, DEF_POOL_SZ); //TODO add it as parameters

   grpIspColorVdoZsl2L_RT.Create(&grpIspColorVdoZsl2L_OS, &RgnAlloc);
   grpIspColorVdoZsl2L_RT.AddTo(&p);
   p.Start();

   DrvLeonRTSignalBootCompleted();
   while(1) { // Add Destroy capability
       if(showCpuReport)
       {
           rtems_cpu_usage_report();
           rtems_cpu_usage_reset();
           showCpuReport = 0;
           sleep(5);
       }
       sleep(1);
   }

   exit(0);
}
