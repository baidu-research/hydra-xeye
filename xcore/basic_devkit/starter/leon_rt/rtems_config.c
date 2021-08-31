///
/// @file      rtems_config.c
/// @copyright All code copyright Movidius Ltd 2017, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     RTEMS configuration Leon header
///            Platform(s) supported : ma2x8x
///

#include <rtems.h>
#include <stdio.h>
#include <OsDrvInit.h>
#include <fatalExtension.h>
/* ask the system to generate a configuration table */
#define CONFIGURE_INIT

#ifndef RTEMS_POSIX_API
#define RTEMS_POSIX_API
#endif

#define CONFIGURE_MICROSECONDS_PER_TICK         1000    /* 1 millisecond */
#define CONFIGURE_TICKS_PER_TIMESLICE           1      /* 10 milliseconds */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_POSIX_INIT_THREAD_TABLE

#define CONFIGURE_MINIMUM_TASK_STACK_SIZE       (4096*10)
#define CONFIGURE_MAXIMUM_TASKS                 30
#define CONFIGURE_MAXIMUM_POSIX_THREADS         50
#define CONFIGURE_MAXIMUM_POSIX_MUTEXES         60
#define CONFIGURE_MAXIMUM_POSIX_KEYS            50
#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES      128
#define CONFIGURE_MAXIMUM_POSIX_CONDITION_VARIABLES 36
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES  36
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES        36
#define CONFIGURE_MAXIMUM_POSIX_TIMERS          36
#define CONFIGURE_MAXIMUM_TIMERS                36
#define CONFIGURE_MAXIMUM_SEMAPHORES            26
#define CONFIGURE_MAXIMUM_DRIVERS               40
#define CONFIGURE_MAXIMUM_DEVICES               40
#define CONFIGURE_MAXIMUM_REGIONS                1

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_FILESYSTEM_DOSFS

#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK

#define CONFIGURE_APPLICATION_NEEDS_ZERO_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_NULL_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 40

#define CONFIGURE_APPLICATION_EXTRA_DRIVERS  OS_DRV_INIT_TABLE_ENTRY

#define CONFIGURE_POSIX_INIT_TASKS_TABLE

void *POSIX_Init (void *args);

#ifdef TASK_SWTCH_LOG_ENABLE
#include "logIspL.h"
#define CONFIGURE_INITIAL_EXTENSIONS         { .fatal = Fatal_extension, .thread_switch = my_thread_switch_extension }
#else
#define CONFIGURE_INITIAL_EXTENSIONS         { .fatal = Fatal_extension }
#endif

#include <rtems/shell.h>
#define CONFIGURE_SHELL_COMMANDS_INIT
#define CONFIGURE_SHELL_COMMANDS_ALL
#include <rtems/shellconfig.h>

#include <rtems/confdefs.h>

/// System Clock configuration on start-up
BSP_SET_CLOCK(DEFAULT_OSC0_KHZ, PLL_DESIRED_FREQ_KHZ, 1, 1, \
           DEFAULT_RTEMS_CSS_LOS_CLOCKS, DEFAULT_RTEMS_MSS_LRT_CLOCKS, DEFAULT_UPA_CLOCKS, 0, 0);


/// Program L2 cache behaviour
BSP_SET_L2C_CONFIG(1,DEFAULT_RTEMS_L2C_REPLACEMENT_POLICY, \
                 DEFAULT_RTEMS_L2C_LOCKED_WAYS,L2C_MODE_WRITE_THROUGH, 0, 0);

