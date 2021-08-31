/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include "DrvI2cDefines.h"
#include "xeye_aec_table.h"
#include "ar0144_common.h"
#include "ar0330_common.h"
#include "xeye_info.h"

#define  __IMAGE_UTILS_NO_DEBUG__

typedef struct AecParameter {
    int minLuma;
    int maxLuma;
    float targetBrightness;
    int minExpo;
    int maxExpo;
    int maxGain;
} AecParam_t;

typedef void (*SET_SENSOR_CB)(I2CM_Device* , int);

const int kMarginRow = 50;
const int kMarginCol = 100;
const int kPixelStep = 2;
static int g_luma_value = 0;
int g_gain;
int g_rows;
static I2CM_Device* sg_i2cHandle = NULL;

AecParam_t aecParam;
SET_SENSOR_CB aec_set_gain_cb;
SET_SENSOR_CB aec_set_expo_cb;


static void gridBrightDarkAdjustBrightness(const unsigned char* raw_img,
        int* adjusted_pixel_val_ptr, int rows, int cols) {
    const int kBrightRegionThres = 240;
    const int kDarkRegionThres = 25;
    const float kBrightRegionWeight = 1.2f;
    const float kDarkRegionWeight = 0.75f;

    //const int kGridSize = 20;
    const int kGridSize = 10;
    const int kPixelsPerGrid = kGridSize * kGridSize / kPixelStep / kPixelStep;

    const int grid_rows = (rows - 2 * kMarginRow) / kGridSize;
    const int grid_cols = (cols - 2 * kMarginCol) / kGridSize;
    int adjusted_pixel_val = 0;

    for (int grid_r = 0; grid_r < grid_rows; ++grid_r) {
        for (int grid_c = 0; grid_c < grid_cols; ++grid_c) {
            int start_row = grid_r * kGridSize + kMarginRow;
            int end_row = start_row + kGridSize;
            int start_col = grid_c * kGridSize + kMarginCol;
            int end_col = start_col + kGridSize;
            int grid_pixel_val = 0;

            for (int i = start_row; i < end_row; i += kPixelStep) {
                for (int j = start_col; j < end_col; j += kPixelStep) {
                    grid_pixel_val += raw_img[i * cols + j];
                }
            }

            grid_pixel_val /= kPixelsPerGrid;

            if (grid_pixel_val > kBrightRegionThres) {
                int tmp = grid_pixel_val * kBrightRegionWeight;
                grid_pixel_val *= kBrightRegionWeight;
                assert(tmp == grid_pixel_val);
            } else if (grid_pixel_val < kDarkRegionThres) {
                int tmp = grid_pixel_val * kDarkRegionWeight;
                grid_pixel_val *= kDarkRegionWeight;
                assert(tmp == grid_pixel_val);
            }

            adjusted_pixel_val += grid_pixel_val;
        }
    }

    *adjusted_pixel_val_ptr = adjusted_pixel_val  / (grid_rows * grid_cols);
}

static int computeNewAecTableIndex(const unsigned char* raw_img, const int smooth_aec,
                                   const unsigned int AEC_steps, int* aec_index_ptr, int rows, int cols) {
    //  XP_CHECK_NOTNULL(aec_index_ptr);
    int  aec_index = *aec_index_ptr;
    //  XP_CHECK_LT(aec_index, AEC_steps);
    //  XP_CHECK_GE(aec_index, 0);

    //  std::vector<int> histogram;
    //  int avg_pixel_val = 0;
    //  int pixel_num = sampleBrightnessHistogram(mono_img, &histogram, &avg_pixel_val);
    //  if (pixel_num == 0) {
    //    // Nothing is sampled.  Something is wrong with raw_image
    //    return false;
    //  }

    int adjusted_pixel_val = 0;
    //gridBrightDarkAdjustBrightness(mono_img, &adjusted_pixel_val);
    gridBrightDarkAdjustBrightness(raw_img, &adjusted_pixel_val, rows, cols);
    //printf("adjusted_pixel_val=%d, AEC_steps=%d  \n",adjusted_pixel_val, AEC_steps );
    g_luma_value = adjusted_pixel_val;

#ifndef __IMAGE_UTILS_NO_DEBUG__
    int acc_pixel_counts = 0;
    int median_pixel_val = 0;

    for (int i = 0; i < 256; ++i) {
        acc_pixel_counts += histogram[i];

        if (acc_pixel_counts >= pixel_num / 2) {
            median_pixel_val = i;
            break;
        }
    }

    int saturate_pixel_counts = 0;

    for (int i = 253; i < 256; ++i) {
        saturate_pixel_counts += histogram[i];
    }

    float saturate_ratio = static_cast<float>(saturate_pixel_counts) / pixel_num;
    XP_VLOG(1, " pixel_val avg = " << avg_pixel_val
            << " adj_avg = " << adjusted_pixel_val
            << " median = " << median_pixel_val
            << " sat_ratio = " << saturate_ratio);
#endif

    // Heuristically adjust AEC table index
    // [NOTE] a step in AEC table is in average ~4% brightness change.  We calculate a rough
    // step number that will drag the adjusted avg_pixel_val close to 128.
    // We simply use add/minus instead multiply/divide here
    // [NOTE] Due to mono aec table, the brightness changes in the first few rows of
    // are very abrupt, e.g., index 0 -> index 1, ratio = 100%
    const float kStepRatioNormal = 0.05f;
    const float kStepRatioBright = 0.10f;
    const float kStepRatioVeryBright = 0.20f;
    const int kMaxStepNumNormal = 5;
    const int kMaxStepNumBright = 2;
    const int kMaxStepNumVeryBright = 1;
    float step_ratio;
    int max_step_num;

    if (aec_index < 16) {
        max_step_num = kMaxStepNumVeryBright;
        step_ratio = kStepRatioVeryBright;
    } else if (aec_index < 32) {
        max_step_num = kMaxStepNumBright;
        step_ratio = kStepRatioBright;
    } else {
        max_step_num = kMaxStepNumNormal;
        step_ratio = kStepRatioNormal;
    }

    // [NOTE] We need to hand tune the target brightness to avoid saturation
    // e.g., 128 can be too high
    float brightness_ratio = 0;

    if (aecParam.targetBrightness > 0) {
        brightness_ratio = adjusted_pixel_val / aecParam.targetBrightness;
    }

    int rough_step_num = (int)((brightness_ratio - 1.f) / step_ratio);

    // If smooth_aec is true, clip the step number to avoid sudden jump in brightness.
    // Otherwise, try to jump directly to the target aec index.
    int actual_step_num;

    if (!smooth_aec) {
        actual_step_num = rough_step_num;
    } else  if (rough_step_num > max_step_num) {
        actual_step_num = max_step_num;
    } else if (rough_step_num < -max_step_num) {
        actual_step_num = -max_step_num;
    } else {
        actual_step_num = rough_step_num;
    }

#ifndef __IMAGE_UTILS_NO_DEBUG__
    //  XP_VLOG(1, " brightness_ratio = " << brightness_ratio
    //          << " rough_step_num = " << rough_step_num
    //          << " actual_step_num = " << actual_step_num);
#endif

    //printf("brightness_ratio=%f,rough_step_num=%d,actual_step_num=%d,aec_index=%d\n",brightness_ratio, rough_step_num, actual_step_num,aec_index );
    // Compute the new aec_index
    const int kLowestAecIndex = 1;
    aec_index -= actual_step_num;

    if (aec_index < kLowestAecIndex) {
        aec_index = kLowestAecIndex;
    } else if (aec_index > AEC_steps - 1) {
        aec_index = AEC_steps - 1;
    }

    //printf("aec_index=%d\n",aec_index);
    *aec_index_ptr = aec_index;
    return 1;
}


//Gray = R*0.299 + G*0.587 + B*0.114
//Gray = (R*76 + G*150 + B*30) >> 8
static void RGBToGray(const unsigned char* r, const unsigned char* g, const unsigned char* b,
                      unsigned char* luma, const int width, const int height) {
    int i = 0;
    int j = 0;
    int index = 0;
    unsigned short m = 0;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            m = r[index] * 76 + g[index] * 150 + b[index] * 30;
            luma[index] = (m >> 8);
            index++;
        }
    }
}

static int UpdateAecExpoAndGain(void) {
    int ret;

    if (g_luma_value >= aecParam.minLuma && g_luma_value <= aecParam.maxLuma) {
        ret = 0;
    } else if (g_luma_value < aecParam.minLuma) {
        if (g_gain < aecParam.maxGain) {
            g_gain += 1;
        } else if (g_rows < aecParam.maxExpo) {
            g_rows += aecParam.minExpo;
        }

        ret = 1;
    } else {
        if (g_gain > 1) {
            g_gain -= 1;
        } else if (g_rows > aecParam.minExpo) {
            g_rows -= aecParam.minExpo;
        }

        ret = 1;
    }

    return ret;
}

// @brief: Initialize aec callback functions and original gain/expo to sensor.
// @input: expo -> should be the minimum of exposure rows
//         gain -> suggest to be minimum of gain
//         XeyeBoardInfo -> get i2c_handle from this param
// @output: none
// @note: This function must be called after xeye_camera_init().
void XeyeDrvAecInit(int expo, int gain, XeyeBoardInfo_t XeyeBoardInfo) {
    sg_i2cHandle = XeyeBoardInfo.camera_i2c_handle;

    // set callback functions
    if (XeyeBoardInfo.sensor_type == Sensor_AR0144) {
        // TODO(hyx): Add code here
        aec_set_gain_cb = AR0144SensorSetGain;
        aec_set_expo_cb = AR0144SensorSetExpo;
    } else if (XeyeBoardInfo.sensor_type == Sensor_AR0330) {
        aec_set_gain_cb = AR0330SensorSetGain;
        aec_set_expo_cb = AR0330SensorSetExpo;
    }

    if (sg_i2cHandle) {
        g_gain = gain;
        g_rows = expo;
        if ((aec_set_gain_cb != NULL) && (aec_set_expo_cb != NULL)) {
            aec_set_gain_cb(sg_i2cHandle, g_gain);
            aec_set_expo_cb(sg_i2cHandle, g_rows);
        }
    }
}

void XeyeDrvAecSetParameter(int minLuma, int maxLuma, float targetBrightness, int maxGain,
                            int minExpo, int maxExpo) {
    aecParam.minLuma = minLuma;
    aecParam.maxLuma = maxLuma;
    aecParam.targetBrightness = targetBrightness;
    aecParam.maxGain = maxGain;
    aecParam.minExpo = minExpo;
    aecParam.maxExpo = maxExpo;
}

void XeyeDrvStartAec(const unsigned char* data, int width, int height) {
    int new_aec_index;

    if (computeNewAecTableIndex(data, 1, kAR0141_AEC_steps, &new_aec_index, height, width)) {
        if (UpdateAecExpoAndGain()) {
            if (sg_i2cHandle) {
                aec_set_gain_cb(sg_i2cHandle, g_gain);
                aec_set_expo_cb(sg_i2cHandle, g_rows);
            }
        }
    }
}