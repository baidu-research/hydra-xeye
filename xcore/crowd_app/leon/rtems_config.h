///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Application configuration Leon header
///

#ifndef _RTEMS_CONFIG_H_
#define _RTEMS_CONFIG_H_

// 1: Includes
// ----------------------------------------------------------------------------
#include <rtems.h>
#include <rtems/bspIo.h>
#include "fatalExtension.h"

// 2: Defines
// ----------------------------------------------------------------------------
#if defined(__RTEMS__)

#if !defined (__CONFIG__)
#define __CONFIG__

/* ask the system to generate a configuration table */
#define CONFIGURE_INIT

#ifndef RTEMS_POSIX_API
#define RTEMS_POSIX_API
#endif

#define CONFIGURE_MICROSECONDS_PER_TICK         1000    /* 1 millisecond */

#define CONFIGURE_TICKS_PER_TIMESLICE           1      /* 1 milliseconds */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_POSIX_INIT_THREAD_TABLE

#define  CONFIGURE_MINIMUM_TASK_STACK_SIZE      8192

#define CONFIGURE_MAXIMUM_TASKS                 16

#define CONFIGURE_MAXIMUM_POSIX_THREADS         16

#define CONFIGURE_MAXIMUM_POSIX_MUTEXES         8

#define CONFIGURE_MAXIMUM_POSIX_KEYS            8

#define CONFIGURE_MAXIMUM_POSIX_SEMAPHORES      16
#define CONFIGURE_MAXIMUM_SEMAPHORES            16
//#define CONFIGURE_MAXIMUM_SEMAPHORES (4 * RTEMS_DOSFS_SEMAPHORES_PER_INSTANCE)
#define CONFIGURE_MAXIMUM_POSIX_MESSAGE_QUEUES  8
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES        16

#define CONFIGURE_MAXIMUM_POSIX_TIMERS          8

#define CONFIGURE_MAXIMUM_TIMERS                8



#define CONFIGURE_MAXIMUM_USER_EXTENSIONS    2
#define CONFIGURE_INITIAL_EXTENSIONS         { .fatal = Fatal_extension }

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_FILESYSTEM_DOSFS

#define CONFIGURE_MAXIMUM_DRIVERS 10


#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 30

#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK

#define CONFIGURE_POSIX_INIT_TASKS_TABLE

#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS   (16)

#define CONFIGURE_BDBUF_MAX_WRITE_BLOCKS        (64)

#define CONFIGURE_BDBUF_BUFFER_MIN_SIZE                 (512)

#define CONFIGURE_BDBUF_BUFFER_MAX_SIZE                 (64 * 1024)

#define CONFIGURE_BDBUF_CACHE_MEMORY_SIZE               (4 * 1024 * 1024)

// include stack checker module to enable runtime task stack overflow checking
#define CONFIGURE_STACK_CHECKER_ENABLED

void* POSIX_Init(void* args);

#include <rtems/confdefs.h>
#endif // __CONFIG__

// Set the system clocks at Startup

#endif // __RTEMS__

#endif // _RTEMS_CONFIG_H_
