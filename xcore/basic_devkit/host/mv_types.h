///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved
///            For License Warranty see: common/license.txt
///
/// @brief     Basic type definitions
///

#ifndef BAIDU_XEYE_MV_TYPES_H
#define BAIDU_XEYE_MV_TYPES_H

#include <stdint.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

typedef uint16_t fp16;
typedef float fp32;

typedef union
{
    uint32_t u32;
    float f32;
} u32f32;

#endif /* BAIDU_XEYE_MV_TYPES_H */
