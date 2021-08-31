///
/// @file
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @defgroup Fp16Convert Fp16 Convert
/// @{
/// @brief Fp16 manipulation and conversion utility
///        minimal set of fp16 conversions functions for
///        sharing data between Leon and SHAVES or other HW blocks
///        which expect fp16 data

#ifndef BAIDU_XEYE_FP16_CONVERT_H
#define BAIDU_XEYE_FP16_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mv_types.h"

#define MOVIDIUS_FP32

#define F32_RND_NEAREST_EVEN     0
#define F32_RND_MINUS_INF        1
#define F32_RND_PLUS_INF         2
#define F32_RND_TO_ZERO          3

#define F32_DETECT_TINY_AFTER_RND  0
#define F32_DETECT_TINY_BEFORE_RND 1

#define F32_EX_INEXACT     0x00000001 //0x00000020
#define F32_EX_DIV_BY_ZERO 0x00000002 //0x00000004
#define F32_EX_INVALID     0x00000004 //0x00000001
#define F32_EX_UNDERFLOW   0x00000008 //0x00000010
#define F32_EX_OVERFLOW    0x00000010 //0x00000008

#define F32_NAN_DEFAULT    0xFFC00000

#define EXTRACT_F16_SIGN(x)   ((x >> 15) & 0x1)
#define EXTRACT_F16_EXP(x)    ((x >> 10) & 0x1F)
#define EXTRACT_F16_FRAC(x)   (x & 0x000003FF)
#define EXTRACT_F32_SIGN(x)   ((x >> 31) & 0x1)
#define EXTRACT_F32_EXP(x)    ((x >> 23) & 0xFF)
#define EXTRACT_F32_FRAC(x)   (x & 0x007FFFFF)
#define RESET_SNAN_BIT(x)     x = x | 0x00400000

#define PACK_F32(x, y, z)     ((x << 31) + (y << 23) + z)
#define PACK_F16(x, y, z)     ((x << 15) + (y << 10) + z)

#define F16_IS_NAN(x)       ((x & 0x7FFF)> 0x7C00)
#define F16_IS_SNAN(x)      (((x & 0x7E00) == 0x7C00)&&((x & 0x1FF)> 0))
#define F32_IS_NAN(x)       ((x & 0x7FFFFFFF)> 0x7F800000)
#define F32_IS_SNAN(x)      (((x & 0x7FC00000) == 0x7F800000)&&((x & 0x3FFFFF)> 0))

unsigned int f32_to_f16(float x);

float f16_to_f32(unsigned int x);

#ifdef __cplusplus
}
#endif

#endif /*BAIDU_XEYE_FP16_CONVERT_H*/
