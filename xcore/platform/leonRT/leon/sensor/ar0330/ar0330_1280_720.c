/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _AR0330_2L_1280x720_RAW10_30Hz_H_
#define _AR0330_2L_1280x720_RAW10_30Hz_H_

#include "mv_types.h"
#include "CameraDefines.h"
#include "ar0330_common.h"

#define AR0330_2L_1280x720_RAW10_30Hz_I2C_CONFIG_STEPS  (3)
// Frame rate Fps = 1/Tframe
// Tframe = 1 /(CLK_PIX) * [frame_length_lines * line_length_pck + extra_delay]
// Minimumframe_length_lines = (y_addr_end - y_addr_start + 1) /
//                             ((y_odd_inc + 1) / 2) + min_vertical_blanking
// Input OSC clock Freq: 24.5Hz
#define OSC_CLK                   24000000
#define PRE_PLL_CLK_DIV            5
#define PLL_MULTIPLIER             100
#define VT_SYS_CLK_DIV             2
#define VT_PIX_CLK_DIV             5
#define OP_SYS_CLK_DIV             1
#define OP_PIX_CLK_DIV             10
#define CLK_PIX            (OSC_CLK / PRE_PLL_CLK_DIV * PLL_MULTIPLIER / OP_SYS_CLK_DIV / OP_PIX_CLK_DIV)
#define FRAME_FPS          30
#define FRAME_LENGTH_LINES 732
#define EXTRA_DELAY        0
#define LINE_LENGTH_PCK    2000
// #define LINE_LENGTH_PCK    ((CLK_PIX / FRAME_FPS -  EXTRA_DELAY)/ FRAME_LENGTH_LINES)

// Exposure/Integration register
#define COARSE_INTEGRATION_TIME       336    // default:16(0X0010)
#define FINE_INTEGRATION_TIME         0       // default:0

#define FLASH_CONTROL         0x0038
#define FLASH_PERIODS         0xFFFF
#define GRR_CONTROL1          0x0081
// Gain relative register
// The digital channel gain format is xxxx.yyyyyyy where
// xxxx refers integer gain of 1 -15
// yyyyyyy refer to fractional gain from 0/128 to 127/128
#define GREEN1_INTEGER        1
#define GREEN1_FRACTIONAL     0
#define GREEN1_GAIN           (GREEN1_INTEGER << 7 | GREEN1_FRACTIONAL)  // default:128(0X80)
#define BLUE_INTEGER          1
#define BLUE_FRACTIONAL       75
#define BLUE_GAIN             (BLUE_INTEGER << 7 | BLUE_FRACTIONAL)      // default:128(0X80)
#define RED_INTEGER           1
#define RED_FRACTIONAL        75
#define RED_GAIN              (RED_INTEGER << 7 | RED_FRACTIONAL)        // default:128(0X80)
#define GREEN2_INTEGER        1
#define GREEN2_FRACTIONAL     0
#define GREEN2_GAIN           (GREEN2_INTEGER << 7 | GREEN2_FRACTIONAL)  // default:128(0X80)
#define GLOBAL_INTEGER        0
#define GLOBAL_FRACTIONAL     0
#define GLOBAL_GAIN           (GLOBAL_INTEGER << 7 | GLOBAL_FRACTIONAL)
// 2^coarse_gain * (1 + fine_gain/16)
#define COARSE_GAIN           2
#define FINE_GAIN             14
#define ANALOG_GAIN           (COARSE_GAIN << 4 | FINE_GAIN)

#define AR0330_WINDOW_X_MAX    2304
#define AR0330_WINDOW_Y_MAX    1536
#define AR0330_IMG_WIDTH       1280
#define AR0330_IMG_HEIGHT      720

#define AR0330_X_ADDR_START    ((AR0330_WINDOW_X_MAX - AR0330_IMG_WIDTH) / 2)
#define AR0330_X_ADDR_END      (AR0330_X_ADDR_START + AR0330_IMG_WIDTH - 1)
#define AR0330_Y_ADDR_START    ((AR0330_WINDOW_Y_MAX - AR0330_IMG_HEIGHT) / 2)
#define AR0330_Y_ADDR_END      (AR0330_Y_ADDR_START + AR0330_IMG_HEIGHT - 1)

#define TEST_PATTERN_MODE 0
#define TEST_DATA_READ    160
#define TEST_DATA_GREENR  200
#define TEST_DATA_BLUE    10
#define TEST_DATA_GREENB  130
//there must be at least 3 standard steps present in the file:
//0)          configure registers set0
//(1), (2)... optionally configure other registers set
//N-1)        set the streaming register (individual step)
//N)          set the standby register (individual step)

const u16 ar0330_2L_1280x720_Raw10_30Hz_I2Cregs[][2] = {
// configuration step 0
    {0x3088, 0x8000},   //  SEQ_CTRL_PORT - [0:00:08.454]
    {0x3086, 0x4A03},   //  SEQ_DATA_PORT - [0:00:08.462]
    {0x3086, 0x4316},   //  SEQ_DATA_PORT - [0:00:08.466]
    {0x3086, 0x0443},   //  SEQ_DATA_PORT - [0:00:08.470]
    {0x3086, 0x1645},   //  SEQ_DATA_PORT - [0:00:08.479]
    {0x3086, 0x4045},   //  SEQ_DATA_PORT - [0:00:08.484]
    {0x3086, 0x6017},   //  SEQ_DATA_PORT - [0:00:08.490]
    {0x3086, 0x2045},   //  SEQ_DATA_PORT - [0:00:08.495]
    {0x3086, 0x404B},   //  SEQ_DATA_PORT - [0:00:08.500]
    {0x3086, 0x1244},   //  SEQ_DATA_PORT - [0:00:08.504]
    {0x3086, 0x6134},   //  SEQ_DATA_PORT - [0:00:08.511]
    {0x3086, 0x4A31},   //  SEQ_DATA_PORT - [0:00:08.520]
    {0x3086, 0x4342},   //  SEQ_DATA_PORT - [0:00:08.526]
    {0x3086, 0x4560},   //  SEQ_DATA_PORT - [0:00:08.532]
    {0x3086, 0x2714},   //  SEQ_DATA_PORT - [0:00:08.540]
    {0x3086, 0x3DFF},   //  SEQ_DATA_PORT - [0:00:08.546]
    {0x3086, 0x3DFF},   //  SEQ_DATA_PORT - [0:00:08.557]
    {0x3086, 0x3DEA},   //  SEQ_DATA_PORT - [0:00:08.562]
    {0x3086, 0x2704},   //  SEQ_DATA_PORT - [0:00:08.567]
    {0x3086, 0x3D10},   //  SEQ_DATA_PORT - [0:00:08.571]
    {0x3086, 0x2705},   //  SEQ_DATA_PORT - [0:00:08.577]
    {0x3086, 0x3D10},   //  SEQ_DATA_PORT - [0:00:08.583]
    {0x3086, 0x2715},   //  SEQ_DATA_PORT - [0:00:08.588]
    {0x3086, 0x3527},   //  SEQ_DATA_PORT - [0:00:08.593]
    {0x3086, 0x053D},   //  SEQ_DATA_PORT - [0:00:08.599]
    {0x3086, 0x1045},   //  SEQ_DATA_PORT - [0:00:08.605]
    {0x3086, 0x4027},   //  SEQ_DATA_PORT - [0:00:08.608]
    {0x3086, 0x0427},   //  SEQ_DATA_PORT - [0:00:08.615]
    {0x3086, 0x143D},   //  SEQ_DATA_PORT - [0:00:08.619]
    {0x3086, 0xFF3D},   //  SEQ_DATA_PORT - [0:00:08.624]
    {0x3086, 0xFF3D},   //  SEQ_DATA_PORT - [0:00:08.630]
    {0x3086, 0xEA62},   //  SEQ_DATA_PORT - [0:00:08.635]
    {0x3086, 0x2728},   //  SEQ_DATA_PORT - [0:00:08.640]
    {0x3086, 0x3627},   //  SEQ_DATA_PORT - [0:00:08.646]
    {0x3086, 0x083D},   //  SEQ_DATA_PORT - [0:00:08.651]
    {0x3086, 0x6444},   //  SEQ_DATA_PORT - [0:00:08.655]
    {0x3086, 0x2C2C},   //  SEQ_DATA_PORT - [0:00:08.662]
    {0x3086, 0x2C2C},   //  SEQ_DATA_PORT - [0:00:08.666]
    {0x3086, 0x4B01},   //  SEQ_DATA_PORT - [0:00:08.671]
    {0x3086, 0x432D},   //  SEQ_DATA_PORT - [0:00:08.676]
    {0x3086, 0x4643},   //  SEQ_DATA_PORT - [0:00:08.681]
    {0x3086, 0x1647},   //  SEQ_DATA_PORT - [0:00:08.686]
    {0x3086, 0x435F},   //  SEQ_DATA_PORT - [0:00:08.692]
    {0x3086, 0x4F50},   //  SEQ_DATA_PORT - [0:00:08.697]
    {0x3086, 0x2604},   //  SEQ_DATA_PORT - [0:00:08.701]
    {0x3086, 0x2684},   //  SEQ_DATA_PORT - [0:00:08.705]
    {0x3086, 0x2027},   //  SEQ_DATA_PORT - [0:00:08.710]
    {0x3086, 0xFC53},   //  SEQ_DATA_PORT - [0:00:08.715]
    {0x3086, 0x0D5C},   //  SEQ_DATA_PORT - [0:00:08.719]
    {0x3086, 0x0D57},   //  SEQ_DATA_PORT - [0:00:08.725]
    {0x3086, 0x5417},   //  SEQ_DATA_PORT - [0:00:08.730]
    {0x3086, 0x0955},   //  SEQ_DATA_PORT - [0:00:08.735]
    {0x3086, 0x5649},   //  SEQ_DATA_PORT - [0:00:08.740]
    {0x3086, 0x5307},   //  SEQ_DATA_PORT - [0:00:08.745]
    {0x3086, 0x5302},   //  SEQ_DATA_PORT - [0:00:08.749]
    {0x3086, 0x4D28},   //  SEQ_DATA_PORT - [0:00:08.755]
    {0x3086, 0x6C4C},   //  SEQ_DATA_PORT - [0:00:08.759]
    {0x3086, 0x0928},   //  SEQ_DATA_PORT - [0:00:08.764]
    {0x3086, 0x2C28},   //  SEQ_DATA_PORT - [0:00:08.771]
    {0x3086, 0x294E},   //  SEQ_DATA_PORT - [0:00:08.776]
    {0x3086, 0x5C09},   //  SEQ_DATA_PORT - [0:00:08.779]
    {0x3086, 0x6045},   //  SEQ_DATA_PORT - [0:00:08.785]
    {0x3086, 0x0045},   //  SEQ_DATA_PORT - [0:00:08.790]
    {0x3086, 0x8026},   //  SEQ_DATA_PORT - [0:00:08.795]
    {0x3086, 0xA627},   //  SEQ_DATA_PORT - [0:00:08.800]
    {0x3086, 0xF817},   //  SEQ_DATA_PORT - [0:00:08.806]
    {0x3086, 0x0227},   //  SEQ_DATA_PORT - [0:00:08.812]
    {0x3086, 0xFA5C},   //  SEQ_DATA_PORT - [0:00:08.817]
    {0x3086, 0x0B17},   //  SEQ_DATA_PORT - [0:00:08.824]
    {0x3086, 0x1826},   //  SEQ_DATA_PORT - [0:00:08.830]
    {0x3086, 0xA25C},   //  SEQ_DATA_PORT - [0:00:08.837]
    {0x3086, 0x0317},   //  SEQ_DATA_PORT - [0:00:08.843]
    {0x3086, 0x4427},   //  SEQ_DATA_PORT - [0:00:08.848]
    {0x3086, 0xF25F},   //  SEQ_DATA_PORT - [0:00:08.854]
    {0x3086, 0x2809},   //  SEQ_DATA_PORT - [0:00:08.859]
    {0x3086, 0x1714},   //  SEQ_DATA_PORT - [0:00:08.865]
    {0x3086, 0x2808},   //  SEQ_DATA_PORT - [0:00:08.869]
    {0x3086, 0x1616},   //  SEQ_DATA_PORT - [0:00:08.874]
    {0x3086, 0x4D1A},   //  SEQ_DATA_PORT - [0:00:08.880]
    {0x3086, 0x2683},   //  SEQ_DATA_PORT - [0:00:08.885]
    {0x3086, 0x1616},   //  SEQ_DATA_PORT - [0:00:08.890]
    {0x3086, 0x27FA},   //  SEQ_DATA_PORT - [0:00:08.896]
    {0x3086, 0x45A0},   //  SEQ_DATA_PORT - [0:00:08.901]
    {0x3086, 0x1707},   //  SEQ_DATA_PORT - [0:00:08.906]
    {0x3086, 0x27FB},   //  SEQ_DATA_PORT - [0:00:08.912]
    {0x3086, 0x1729},   //  SEQ_DATA_PORT - [0:00:08.917]
    {0x3086, 0x4580},   //  SEQ_DATA_PORT - [0:00:08.921]
    {0x3086, 0x1708},   //  SEQ_DATA_PORT - [0:00:08.927]
    {0x3086, 0x27FA},   //  SEQ_DATA_PORT - [0:00:08.932]
    {0x3086, 0x1728},   //  SEQ_DATA_PORT - [0:00:08.937]
    {0x3086, 0x5D17},   //  SEQ_DATA_PORT - [0:00:08.943]
    {0x3086, 0x0E26},   //  SEQ_DATA_PORT - [0:00:08.948]
    {0x3086, 0x8153},   //  SEQ_DATA_PORT - [0:00:08.952]
    {0x3086, 0x0117},   //  SEQ_DATA_PORT - [0:00:08.959]
    {0x3086, 0xE653},   //  SEQ_DATA_PORT - [0:00:08.963]
    {0x3086, 0x0217},   //  SEQ_DATA_PORT - [0:00:08.967]
    {0x3086, 0x1026},   //  SEQ_DATA_PORT - [0:00:08.973]
    {0x3086, 0x8326},   //  SEQ_DATA_PORT - [0:00:08.977]
    {0x3086, 0x8248},   //  SEQ_DATA_PORT - [0:00:08.981]
    {0x3086, 0x4D4E},   //  SEQ_DATA_PORT - [0:00:08.985]
    {0x3086, 0x2809},   //  SEQ_DATA_PORT - [0:00:08.991]
    {0x3086, 0x4C0B},   //  SEQ_DATA_PORT - [0:00:08.995]
    {0x3086, 0x6017},   //  SEQ_DATA_PORT - [0:00:08.999]
    {0x3086, 0x2027},   //  SEQ_DATA_PORT - [0:00:09.005]
    {0x3086, 0xF217},   //  SEQ_DATA_PORT - [0:00:09.011]
    {0x3086, 0x535F},   //  SEQ_DATA_PORT - [0:00:09.016]
    {0x3086, 0x2808},   //  SEQ_DATA_PORT - [0:00:09.022]
    {0x3086, 0x164D},   //  SEQ_DATA_PORT - [0:00:09.027]
    {0x3086, 0x1A16},   //  SEQ_DATA_PORT - [0:00:09.032]
    {0x3086, 0x1627},   //  SEQ_DATA_PORT - [0:00:09.037]
    {0x3086, 0xFA26},   //  SEQ_DATA_PORT - [0:00:09.042]
    {0x3086, 0x035C},   //  SEQ_DATA_PORT - [0:00:09.047]
    {0x3086, 0x0145},   //  SEQ_DATA_PORT - [0:00:09.053]
    {0x3086, 0x4027},   //  SEQ_DATA_PORT - [0:00:09.058]
    {0x3086, 0x9817},   //  SEQ_DATA_PORT - [0:00:09.062]
    {0x3086, 0x2A4A},   //  SEQ_DATA_PORT - [0:00:09.068]
    {0x3086, 0x0A43},   //  SEQ_DATA_PORT - [0:00:09.072]
    {0x3086, 0x160B},   //  SEQ_DATA_PORT - [0:00:09.077]
    {0x3086, 0x4327},   //  SEQ_DATA_PORT - [0:00:09.082]
    {0x3086, 0x9C45},   //  SEQ_DATA_PORT - [0:00:09.086]
    {0x3086, 0x6017},   //  SEQ_DATA_PORT - [0:00:09.090]
    {0x3086, 0x0727},   //  SEQ_DATA_PORT - [0:00:09.095]
    {0x3086, 0x9D17},   //  SEQ_DATA_PORT - [0:00:09.099]
    {0x3086, 0x2545},   //  SEQ_DATA_PORT - [0:00:09.104]
    {0x3086, 0x4017},   //  SEQ_DATA_PORT - [0:00:09.109]
    {0x3086, 0x0827},   //  SEQ_DATA_PORT - [0:00:09.114]
    {0x3086, 0x985D},   //  SEQ_DATA_PORT - [0:00:09.119]
    {0x3086, 0x2645},   //  SEQ_DATA_PORT - [0:00:09.123]
    {0x3086, 0x5C01},   //  SEQ_DATA_PORT - [0:00:09.127]
    {0x3086, 0x4B17},   //  SEQ_DATA_PORT - [0:00:09.133]
    {0x3086, 0x0A28},   //  SEQ_DATA_PORT - [0:00:09.137]
    {0x3086, 0x0853},   //  SEQ_DATA_PORT - [0:00:09.143]
    {0x3086, 0x0D52},   //  SEQ_DATA_PORT - [0:00:09.149]
    {0x3086, 0x5112},   //  SEQ_DATA_PORT - [0:00:09.154]
    {0x3086, 0x4460},   //  SEQ_DATA_PORT - [0:00:09.164]
    {0x3086, 0x184A},   //  SEQ_DATA_PORT - [0:00:09.177]
    {0x3086, 0x0343},   //  SEQ_DATA_PORT - [0:00:09.183]
    {0x3086, 0x1604},   //  SEQ_DATA_PORT - [0:00:09.189]
    {0x3086, 0x4316},   //  SEQ_DATA_PORT - [0:00:09.193]
    {0x3086, 0x5843},   //  SEQ_DATA_PORT - [0:00:09.198]
    {0x3086, 0x1659},   //  SEQ_DATA_PORT - [0:00:09.203]
    {0x3086, 0x4316},   //  SEQ_DATA_PORT - [0:00:09.208]
    {0x3086, 0x5A43},   //  SEQ_DATA_PORT - [0:00:09.213]
    {0x3086, 0x165B},   //  SEQ_DATA_PORT - [0:00:09.218]
    {0x3086, 0x4345},   //  SEQ_DATA_PORT - [0:00:09.224]
    {0x3086, 0x4027},   //  SEQ_DATA_PORT - [0:00:09.228]
    {0x3086, 0x9C45},   //  SEQ_DATA_PORT - [0:00:09.233]
    {0x3086, 0x6017},   //  SEQ_DATA_PORT - [0:00:09.237]
    {0x3086, 0x0727},   //  SEQ_DATA_PORT - [0:00:09.243]
    {0x3086, 0x9D17},   //  SEQ_DATA_PORT - [0:00:09.248]
    {0x3086, 0x2545},   //  SEQ_DATA_PORT - [0:00:09.254]
    {0x3086, 0x4017},   //  SEQ_DATA_PORT - [0:00:09.258]
    {0x3086, 0x1027},   //  SEQ_DATA_PORT - [0:00:09.263]
    {0x3086, 0x9817},   //  SEQ_DATA_PORT - [0:00:09.269]
    {0x3086, 0x2022},   //  SEQ_DATA_PORT - [0:00:09.274]
    {0x3086, 0x4B12},   //  SEQ_DATA_PORT - [0:00:09.279]
    {0x3086, 0x442C},   //  SEQ_DATA_PORT - [0:00:09.284]
    {0x3086, 0x2C2C},   //  SEQ_DATA_PORT - [0:00:09.289]
    {0x3086, 0x2C00},   //  SEQ_DATA_PORT - [0:00:09.294]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.299]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.305]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.310]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.317]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.323]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.327]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.333]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.337]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.342]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.348]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.353]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.358]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.364]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.369]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.374]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.379]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.384]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.389]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.396]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.401]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.408]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.415]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.420]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.426]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.432]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.437]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.443]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.448]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.458]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.465]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.470]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.477]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.483]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.490]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.495]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.503]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.511]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.523]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.531]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.539]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.546]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.554]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.561]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.568]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.577]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.583]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.589]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.595]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.605]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.609]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.619]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.630]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.640]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.652]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.659]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.666]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.673]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.681]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.686]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.691]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.697]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.705]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.715]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.723]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.731]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.737]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.742]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.747]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.751]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.759]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.764]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.770]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.776]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.780]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.785]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.791]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.797]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.802]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.808]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.815]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.821]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.826]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.833]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.839]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.844]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.850]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.855]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.866]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.871]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.877]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.881]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.886]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.894]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.900]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.915]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.926]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.931]
    {0x3086, 0x0000},   //  SEQ_DATA_PORT - [0:00:09.958]
    {0x3088, 0x800C},   //  SEQ_CTRL_PORT - [0:00:10.134]
    {0x3086, 0x2045},   //  SEQ_DATA_PORT - [0:00:10.141]
//-------------------------------

//[AR0330 Register Wizard output preset]

//PLL_settings
    {0x302E, PRE_PLL_CLK_DIV},   //  PRE_PLL_CLK_DIV
    {0x3030, PLL_MULTIPLIER},    //  PLL_MULTIPLIER
    {0x302C, VT_SYS_CLK_DIV},    //  VT_SYS_CLK_DIV
    {0x302A, VT_PIX_CLK_DIV},    //  VT_PIX_CLK_DIV
    {0x3038, OP_SYS_CLK_DIV},    //  OP_SYS_CLK_DIV
    {0x3036, OP_PIX_CLK_DIV},    //  OP_PIX_CLK_DIV
    {0x31AC, 0x0A0A},   //  DATA_FORMAT_BITS
    {0x31AE, 0x0202},   //  SERIAL_FORMAT

//MIPI Port Timing
    {0x31B0, 0x0038},   //  FRAME_PREAMBLE
    {0x31B2, 0x0016},   //  LINE_PREAMBLE
    {0x31B4, 0x3E55},   //  MIPI_TIMING_0
    {0x31B6, 0x41D1},   //  MIPI_TIMING_1
    {0x31B8, 0x208B},   //  MIPI_TIMING_2
    {0x31BA, 0x0288},   //  MIPI_TIMING_3
    {0x31BC, 0x8007},   //  MIPI_TIMING_4

//Timing_settings
    {0x3002, AR0330_Y_ADDR_START},   //  Y_ADDR_START
    {0x3004, AR0330_X_ADDR_START},   //  X_ADDR_START
    {0x3006, AR0330_Y_ADDR_END},     //  Y_ADDR_END
    {0x3008, AR0330_X_ADDR_END},     //  X_ADDR_END
    {0x300A, FRAME_LENGTH_LINES},    //  FRAME_LENGTH_LINES
    {0x300C, LINE_LENGTH_PCK},       //  LINE_LENGTH_PCK
    {0x3012, COARSE_INTEGRATION_TIME},   //  COARSE_INTEGRATION_TIME
    {0x3014, FINE_INTEGRATION_TIME},     //  FINE_INTEGRATION_TIME
    {0x30A2, 0x0001},   //  X_ODD_INC
    {0x30A6, 0x0001},   //  Y_ODD_INC
    {0x308C, 0x0006},   //  Y_ADDR_START_CB
    {0x308A, 0x0006},   //  X_ADDR_START_CB
    {0x3090, 0x0605},   //  Y_ADDR_END_CB
    {0x308E, 0x0905},   //  X_ADDR_END_CB
    {0x30AA, 0x060C},   //  FRAME_LENGTH_LINES_CB
    {0x303E, 0x04E0},   //  LINE_LENGTH_PCK_CB
    {0x3016, 0x060B},   //  COARSE_INTEGRATION_TIME_CB
    {0x3018, 0x0000},   //  FINE_INTEGRATION_TIME_CB
    {0x30AE, 0x0001},   //  X_ODD_INC_CB
    {0x30A8, 0x0001},   //  Y_ODD_INC_CB
    {0x3040, 0x0000},   //  READ_MODE
    {0x3042, EXTRA_DELAY},   //  EXTRA_DELAY
    {0x30B0, 0x8000},  //  DIGITAL_TEST
    {0x30BA, 0x002C},   //  DIGITAL_CTRL

//Recommended Configuration

//Test pattern mode
    {0x3072, TEST_DATA_READ},    //  Test red pixels
    {0x3074, TEST_DATA_GREENR},  //  Test greenr pixels
    {0x3076, TEST_DATA_BLUE},    //  Test blue pixels
    {0x3078, TEST_DATA_GREENB},  //  Test greeb pixels
    {0x3070, TEST_PATTERN_MODE}, //  Test pattern mode

    {0x31E0, 0x0303},
    {0x3064, 0x1802},
    {0x3ED2, 0x0146},
    {0x3ED4, 0x8F6C},
    {0x3ED6, 0x66CC},
    {0x3ED8, 0x8C42},
    {0x3EDA, 0x88BC},
    {0x3EDC, 0xAA63},
    {0x305E, 0x00A0},

    {0x3056, GREEN1_GAIN},   //  GREEN1_GAIN
    {0x3058, BLUE_GAIN},     //  BLUE_GAIN
    {0x305A, RED_GAIN},      //  RED_GAIN
    {0x305C, GREEN2_GAIN},   //  GREEN2_GAIN
    {0x3060, ANALOG_GAIN},   //  ANALOG_GAIN

    {0x301A, 0x005C},  //  Enable streaming
    {0x301A, 0x0058}  // Disable streaming
};

I2CConfigDescriptor ar0330_2L_1280x720_Raw10_30Hz_mipiCfg_i2cConfigSteps[AR0330_2L_1280x720_RAW10_30Hz_I2C_CONFIG_STEPS] =
{
    {   .numberOfRegs = (sizeof(ar0330_2L_1280x720_Raw10_30Hz_I2Cregs)/sizeof(ar0330_2L_1280x720_Raw10_30Hz_I2Cregs[0]) -2),
        .delayMs = 10,
    },
    {   .numberOfRegs = 1, //the streaming configuration step
        .delayMs = 10
    },
    {   .numberOfRegs = 1, //the standby configuration step
        .delayMs = 10
    }
};

const mipiSpec ar0330_2L_1280x720_Raw10_30Hz_mipiCfg =
{
    .dataMode     = MIPI_D_MODE_0,
    .dataRateMbps = 1200,
    .nbOflanes    = 2,
    .pixelFormat  = CSI_RAW_10
};

GenericCamSpec ar0330_2L_1280x720_Raw10_30Hz_camCfg =
{
    .frameWidth            = 1280,
    .frameHeight           = 720,
    .hBackPorch            = 0,
    .hFrontPorch           = 0,
    .vBackPorch            = 0,
    .vFrontPorch           = 0,
    .bytesPerPixel         = 2,
    .internalPixelFormat   = RAW16,
    .mipiCfg               = &ar0330_2L_1280x720_Raw10_30Hz_mipiCfg,
    .idealRefFreq          = 24,
    .sensorI2CAddress1     = AR0330_I2C_ADDRESS,
    .sensorI2CAddress2     = 0,             //only used for stereo cameras
    .nbOfI2CConfigSteps    = AR0330_2L_1280x720_RAW10_30Hz_I2C_CONFIG_STEPS,
    .i2cConfigSteps        = ar0330_2L_1280x720_Raw10_30Hz_mipiCfg_i2cConfigSteps,
    .regSize               = 2,
    .regValues             = ar0330_2L_1280x720_Raw10_30Hz_I2Cregs,
};

#endif  // _AR0330_2L_1280x720_RAW10_30Hz_H_