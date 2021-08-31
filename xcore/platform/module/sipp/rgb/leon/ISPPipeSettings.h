/// ISPPipeSettings.h


#ifndef _ISP_PIPE_SETTINGS_H_
#define _ISP_PIPE_SETTINGS_H_

///////////////////////////////////////////
// General

///////////////////////////////////////////
// Input image dimensions:
//#define ispc_bayer_bits 10 //Moved to specific sensor driver
#define BAYER_PATTERN   RGGB
#define FORMAT          BAYER
#define SIPP_ISP_PIPE_BUFFER_INPUT_NUM_PLANES 1
//IMX378
#define SIPP_ISP_PIPE_BUFFER_WIDTH_IMX378     4032
#define SIPP_ISP_PIPE_BUFFER_HEIGHT_IMX378    2376
#define FULL_IMAGE_WIDTH_IMX378   SIPP_ISP_PIPE_BUFFER_WIDTH_IMX378
#define FULL_IMAGE_HEIGHT_IMX378  SIPP_ISP_PIPE_BUFFER_HEIGHT_IMX378
#define ISPC_CHROMA_PIPE_WIDTH_IMX378    (SIPP_ISP_PIPE_BUFFER_WIDTH_IMX378 >> 0x1)
#define ISPC_CHROMA_PIPE_HEIGHT_IMX378   (SIPP_ISP_PIPE_BUFFER_HEIGHT_IMX378 >> 0x1)
//IMX214
#define SIPP_ISP_PIPE_BUFFER_WIDTH_IMX214  1920
#define SIPP_ISP_PIPE_BUFFER_HEIGHT_IMX214 1080
#define FULL_IMAGE_WIDTH_IMX214   SIPP_ISP_PIPE_BUFFER_WIDTH_IMX214
#define FULL_IMAGE_HEIGHT_IMX214  SIPP_ISP_PIPE_BUFFER_HEIGHT_IMX214
#define ISPC_CHROMA_PIPE_WIDTH_IMX214  (FULL_IMAGE_WIDTH_IMX214 >> 0x1)
#define ISPC_CHROMA_PIPE_HEIGHT_IMX214 (FULL_IMAGE_HEIGHT_IMX214 >> 0x1)

#define SIPP_ISP_PIPE_BUFFER_WIDTH  RES_WIDTH
#define SIPP_ISP_PIPE_BUFFER_HEIGHT RES_HEIGHT
#define FULL_IMAGE_WIDTH   SIPP_ISP_PIPE_BUFFER_WIDTH
#define FULL_IMAGE_HEIGHT  SIPP_ISP_PIPE_BUFFER_HEIGHT
#define ISPC_CHROMA_PIPE_WIDTH  (FULL_IMAGE_WIDTH >> 0x1)
#define ISPC_CHROMA_PIPE_HEIGHT (FULL_IMAGE_HEIGHT >> 0x1)
///////////////////////////////////////////
// HW Filters

///////////////////////////////////////////
// Sigma denoise filter

#define ISPC_SIGMA_NOISE_FLOOR  51
#define ISPC_SIGMA_THRESH1_P0   9
#define ISPC_SIGMA_THRESH2_P0   18
#define ISPC_SIGMA_THRESH1_P1   9
#define ISPC_SIGMA_THRESH2_P1   18
#define ISPC_SIGMA_THRESH1_P2   9
#define ISPC_SIGMA_THRESH2_P2   18
#define ISPC_SIGMA_THRESH1_P3   9
#define ISPC_SIGMA_THRESH2_P3   18

#define SIGMA_DNS_PASSTHRU_BIT  DISABLED


///////////////////////////////////////////
// LSC filter

#define ISPC_LSC_GAIN_MAP_W 60
#define ISPC_LSC_GAIN_MAP_H 44

extern uint16_t ispcLscMesh[ISPC_LSC_GAIN_MAP_W* ISPC_LSC_GAIN_MAP_H];

///////////////////////////////////////////
// Raw filter

// Bad pixel suppression
#define ISPC_BAD_PIX_ALPHA_G_HOT    0x6 // 4 bits
#define ISPC_BAD_PIX_ALPHA_RB_HOT   0x6 // 4 bits
#define ISPC_BAD_PIX_ALPHA_G_COLD   0x6 // 4 bits
#define ISPC_BAD_PIX_ALPHA_RB_COLD  0x6 // 4 bits
#define ISPC_BAD_PIX_NOISE_LEVEL    0x0  //16 bits

// GR-GB imbalance filter
#define ISPC_GRGB_IMBAL_PLAT_DARK    144
#define ISPC_GRGB_IMBAL_DECAY_DARK   160
#define ISPC_GRGB_IMBAL_PLAT_BRIGHT  200
#define ISPC_GRGB_IMBAL_DECAY_BRIGHT 240
#define ISPC_GRGB_IMBAL_THRESHOLD    1000    // 8 bits

#define ISPC_RAW_GAIN_R_IMX378     1948    // 16 bits
#define ISPC_RAW_GAIN_GR_IMX378    730     // 16 bits
#define ISPC_RAW_GAIN_GB_IMX378    888     // 16 bits
#define ISPC_RAW_GAIN_B_IMX378     1873    // 16 bits

#define ISPC_RAW_GAIN_R_IMX214     0x0100    // 16 bits
#define ISPC_RAW_GAIN_GR_IMX214    0x0070    // 16 bits
#define ISPC_RAW_GAIN_GB_IMX214    0x0090    // 16 bits
#define ISPC_RAW_GAIN_B_IMX214     0x0100    // 16 bits

#define ISPC_RAW_GAIN_R_AR0330     300    // 16 bits
#define ISPC_RAW_GAIN_GR_AR0330    300    // 16 bits
#define ISPC_RAW_GAIN_GB_AR0330    300    // 16 bits
#define ISPC_RAW_GAIN_B_AR0330     300    // 16 bits

#define ISPC_RAW_GAIN_R_DEFAULT    0x0010    // 16 bits
#define ISPC_RAW_GAIN_GR_DEFAULT   0x0010    // 16 bits
#define ISPC_RAW_GAIN_GB_DEFAULT   0x0010    // 16 bits
#define ISPC_RAW_GAIN_B_DEFAULT    0x0010    // 16 bits


#define GRGB_IMBAL_EN            ENABLED
#define AE_PATCH_STATS_EN        DISABLED
#define AF_PATCH_STATS_EN        DISABLED
#define AE_Y_HIST_STATS_EN       ENABLED
#define AE_RGB_HIST_STATS_EN     DISABLED
#define HOT_COLD_PIX_SUPPRESS_EN ENABLED
#define BAYER_2x2_MODE           1

///////////////////////////////////////////
// Debayer filter

#define RGB_EN                   1
#define FORCE_RB_ZERO            0
#define IMAGE_ORDER_OUT          P_RGB
#define OUTPUT_PLANE_NO          3

#define ISPC_DEMOSAIC_MIX_SLOPE            0
#define ISPC_DEMOSAIC_MIX_OFFSET           0
#define ISPC_DEMOSAIC_MIX_GRADIENT_MUL     0
#define ISPC_DEMOSAIC_LUMA_WEIGHT_RED     77
#define ISPC_DEMOSAIC_LUMA_WEIGHT_GREEN  150
#define ISPC_DEMOSAIC_LUMA_WEIGHT_BLUE    29

///////////////////////////////////////////
// DoGLTM filter

#define MODE_DOG_ONLY     0x0
#define MODE_LTM_ONLY     0x1
#define MODE_DOG_DENOISE  0x2
#define MODE_DOG_LTM      0x3
#define DOG_LTM_MODE      MODE_DOG_LTM

#define DOG_OUTPUT_CLAMP  DISABLED
#define DOG_HEIGHT        15

extern float ispcLtmCurves[16 * 8];

// Pre-filter for Local Tone Mapping:
#define ISPC_LTM_FILTER_TH1    35

// DoG Denoise params:
#define ISPC_DOG_THR           4
#define ISPC_DOG_STRENGTH      191

extern uint8_t dogCoeffs11[6];
extern uint8_t dogCoeffs15[8];
extern u16 ltm_curves[16 * 8];

#define DSMODE_CARRY 0x0
#define DSMODE_DOWN  0x1
#define DSMODE_UPDN  0x2
#define DS_MODE      DSMODE_DOWN

///////////////////////////////////////////
// Luma denoise filter

// The following coefs are mirrored by the HW to obtain 7x7 matrix
// The 16 coefs represent the top-left square of a 7x7 symetrical matrix
// Each entry is 2 bits, and represents a left shift applied to
// the weight at the corresponding location.
#define ISPC_LUMA_DNS_F2         0xaaaaa9a4
#define ISPC_LUMA_DNS_BITPOS     7

extern uint8_t ispcLumaDnsLut[32];

extern uint8_t ispcYDnsDistLut[256];

extern uint8_t ispcGammaLut0_32[9];
extern uint8_t ispcGammaLut32_255[9];

#define ISPC_LUMA_DNS_REF_SHIFT 15
#define X_OFFSET                0
#define Y_OFFSET                0

extern u32 gaussLut[4];
extern u32 gammaLut[5];

#define ISPC_LUMA_DNS_STRENGTH  150.000000
#define ISPC_LUMA_DNS_ALPHA     128 // 8bit value

///////////////////////////////////////////
// Sharpen filter

#define SHARPEN_KERNEL_SIZE         7

#define SHARPEN_STRENGTH_DARKEN     0x0000   // 0.7f
#define SHARPEN_STRENGTH_LIGHTEN    0x0000   // 1.3f
#define SHARPEN_CLIPPING_ALPHA      0x0000   // 1.0f
#define SHARPEN_RANGE_STOP0         0x0000
#define SHARPEN_RANGE_STOP1         0x0000
#define SHARPEN_RANGE_STOP2         0x0000
#define SHARPEN_RANGE_STOP3         0x0000
#define SHARPENING                  0
#define BLURRING                    1
#define SHARPEN_MODE                SHARPENING
#define OUTPUT_DELTAS               0
#define THR                         0x0
#define OUTPUT_CLAMP                1


#define ISPC_SHARP_SIGMA            2.000000
//#define ISPC_SHARP_STRENGTH_DARKEN  0.800000
//#define ISPC_SHARP_STRENGTH_LIGHTEN 2.000000
#define ISPC_SHARP_ALPHA            0.700000
#define ISPC_SHARP_OVERSHOOT        1.050000
#define ISPC_SHARP_UNDERSHOOT       1.000000
#define ISPC_SHARP_RANGE_STOP_0     0.003922
#define ISPC_SHARP_RANGE_STOP_1     0.019608
#define ISPC_SHARP_RANGE_STOP_2     0.980392
#define ISPC_SHARP_RANGE_STOP_3     1.000000
#define ISPC_SHARP_MIN_THR          0.000000
#define ISPC_SHARP_COEF0            0.070159
#define ISPC_SHARP_COEF1            0.131075
#define ISPC_SHARP_COEF2            0.190713
#define ISPC_SHARP_COEF3            0.216106


///////////////////////////////////////////
// Chroma gen filter

#define ISPC_CGEN_EPSILON               1
#define ISPC_CGEN_KR                    106
#define ISPC_CGEN_KG                    191
#define ISPC_CGEN_KB                    149
#define ISPC_CGEN_LUMA_COEFF_R          76
#define ISPC_CGEN_LUMA_COEFF_G          150
#define ISPC_CGEN_LUMA_COEFF_B          29
#define ISPC_CGEN_PFR_STRENGTH          80
#define ISPC_CGEN_DESAT_OFFSET          4
#define ISPC_CGEN_DESAT_SLOPE           43

#define BYPASS_PF_DAD       DISABLED
#define DSMODE_CARRY        0x0
#define DSMODE_DOWN         0x1
#define DSMODE_UPDN         0x2
#define DS_MODE             DSMODE_DOWN

///////////////////////////////////////////
// Median filter

#define MEDIAN_KERNEL_SIZE         0x7                 // Check this - should be passed to link filter?
#define MEDIAN_THRESHOLD           0x1FF
#define MEDIAN_OUT_SEL             ((MEDIAN_KERNEL_SIZE >> 1) * (MEDIAN_KERNEL_SIZE) + (MEDIAN_KERNEL_SIZE >> 1))
#define MEDIAN_LUMA_ABLEND_EN      ENABLED
#define MEDIAN_LUMA_SAMPLE_EN      ENABLED

#define ISPC_CHROMA_MEDIAN_MIX_SLOPE    10
#define ISPC_CHROMA_MEDIAN_MIX_OFFSET   -26


///////////////////////////////////////////
// Chroma Dns filter
#define ISPC_CHROMA_DNS_TH_R    20
#define ISPC_CHROMA_DNS_TH_G    15
#define ISPC_CHROMA_DNS_TH_B    38
#define ISPC_CHROMA_DNS_LIMIT   100
#define ISPC_CHROMA_DNS_H_ENAB  7
#define PLANES_PER_CYCLE        3
#define FORCE_WT_H          0
#define FORCE_WT_V          0

// Grey desaturate params:
#define ISPC_GREY_DESAT_OFFSET -35
#define ISPC_GREY_DESAT_SLOPE  7
#define ISPC_GREY_POINT_R      106
#define ISPC_GREY_POINT_G      137
#define ISPC_GREY_POINT_B      75
#define GREY_PT_EN             DISABLED

extern uint8_t ispcLowpassKernel[9];

#define ISPC_CHROMA_DNS_TH_R    20
#define ISPC_CHROMA_DNS_TH_G    15
#define ISPC_CHROMA_DNS_TH_B    38

#define CHROMA_DNS_THRESH0     (ISPC_CHROMA_DNS_TH_R | (ISPC_CHROMA_DNS_TH_R << 18) | (ISPC_CHROMA_DNS_TH_G << 10) | (ISPC_CHROMA_DNS_TH_G << 26))
#define CHROMA_DNS_THRESH1     (ISPC_CHROMA_DNS_TH_B | (ISPC_CHROMA_DNS_TH_B << 18))

///////////////////////////////////////////
// Color Combination filter
extern float ispcCCM[9];
extern float ispcCCMOff[3];

#define FORCE_LUMA_ONE      DISABLED
#define LUT_3D_BYPASS       ENABLED
#define LUT_3D_LOAD         DISABLED
#define U12F_OUTPUT         DISABLED
#define MUL                 255
#define CC_OUTPUT_PLANE     3

#define T1                  2

#define ISPC_CC_KR      460
#define ISPC_CC_KG      343
#define ISPC_CC_KB      355

extern u16 ccm_lut_coeffs[9];
extern u16 ccm_lut_offsets[3];

///////////////////////////////////////////
// LUT filter

#define INTERP_MODE         ENABLED
#define CHANNEL_MODE        ENABLED
#define INT_WIDTH           12
#define LUTS_NO             2
#define LUT_LOAD            ENABLED
#define APB_ACCESS          DISABLED
#define CSC_ENABLE          ENABLED
#define CHANNELS_NO         3

extern float ispcGammaTable[8192];
extern uint16_t gammaLutFp16[512 * 4];
extern float ispcCSC[9];
extern float ispcCSCOff[3];

extern uint16_t lut3d[16 * 16 * 16 * 4];

extern u16 csc_lut_coeffs[9];
extern u16 csc_lut_offsets[3];

#define LUT_LD_FORMAT       0

#define LUT_MAT_11          csc_lut_coeffs[0]
#define LUT_MAT_12          csc_lut_coeffs[1]
#define LUT_MAT_13          csc_lut_coeffs[2]
#define LUT_MAT_21          csc_lut_coeffs[3]
#define LUT_MAT_22          csc_lut_coeffs[4]
#define LUT_MAT_23          csc_lut_coeffs[5]
#define LUT_MAT_31          csc_lut_coeffs[6]
#define LUT_MAT_32          csc_lut_coeffs[7]
#define LUT_MAT_33          csc_lut_coeffs[8]
#define LUT_OFF_1           csc_lut_offsets[0]
#define LUT_OFF_2           csc_lut_offsets[1]
#define LUT_OFF_3           csc_lut_offsets[2]

#define LUT_REGION0_SIZE_I  5
#define LUT_REGION1_SIZE_I  5
#define LUT_REGION2_SIZE_I  5
#define LUT_REGION3_SIZE_I  5
#define LUT_REGION4_SIZE_I  5
#define LUT_REGION5_SIZE_I  5
#define LUT_REGION6_SIZE_I  5
#define LUT_REGION7_SIZE_I  5

#define LUT_REGION8_SIZE_I  5
#define LUT_REGION9_SIZE_I  5
#define LUT_REGION10_SIZE_I 5
#define LUT_REGION11_SIZE_I 5
#define LUT_REGION12_SIZE_I 5
#define LUT_REGION13_SIZE_I 5
#define LUT_REGION14_SIZE_I 5
#define LUT_REGION15_SIZE_I 5

///////////////////////////////////////////
// PolyFIR filters

#define POLYFIR_Y_H_NUM_IMX378   10
#define POLYFIR_Y_H_DEN_IMX378   21
#define POLYFIR_Y_V_NUM_IMX378   5
#define POLYFIR_Y_V_DEN_IMX378   11
#define POLYFIR_U_H_NUM_IMX378   5
#define POLYFIR_U_H_DEN_IMX378   21
#define POLYFIR_U_V_NUM_IMX378   5
#define POLYFIR_U_V_DEN_IMX378   22
#define POLYFIR_V_H_NUM_IMX378   5
#define POLYFIR_V_H_DEN_IMX378   21
#define POLYFIR_V_V_NUM_IMX378   5
#define POLYFIR_V_V_DEN_IMX378   22

#define POLYFIR_Y_H_NUM_IMX214   1
#define POLYFIR_Y_H_DEN_IMX214   1
#define POLYFIR_Y_V_NUM_IMX214   1
#define POLYFIR_Y_V_DEN_IMX214   1
#define POLYFIR_U_H_NUM_IMX214   1
#define POLYFIR_U_H_DEN_IMX214   2
#define POLYFIR_U_V_NUM_IMX214   1
#define POLYFIR_U_V_DEN_IMX214   2
#define POLYFIR_V_H_NUM_IMX214   1
#define POLYFIR_V_H_DEN_IMX214   2
#define POLYFIR_V_V_NUM_IMX214   1
#define POLYFIR_V_V_DEN_IMX214   2

#define POLYFIR_Y_H_NUM   1
#define POLYFIR_Y_H_DEN   1
#define POLYFIR_Y_V_NUM   1
#define POLYFIR_Y_V_DEN   1
#define POLYFIR_U_H_NUM   1
#define POLYFIR_U_H_DEN   2
#define POLYFIR_U_V_NUM   1
#define POLYFIR_U_V_DEN   2
#define POLYFIR_V_H_NUM   1
#define POLYFIR_V_H_DEN   2
#define POLYFIR_V_V_NUM   1
#define POLYFIR_V_V_DEN   2


extern uint8_t hCoefs[]; //up to 10 phases
extern uint8_t vCoefs[];

#define POLY_Y_IMAGE_WIDTH_IMX378    ((((FULL_IMAGE_WIDTH_IMX378 * POLYFIR_Y_H_NUM_IMX378) - 1) / POLYFIR_Y_H_DEN_IMX378) + 1)
#define POLY_Y_IMAGE_HEIGHT_IMX378   ((((FULL_IMAGE_HEIGHT_IMX378 * POLYFIR_Y_V_NUM_IMX378) - 1) / POLYFIR_Y_V_DEN_IMX378) + 1)
#define POLY_UV_IMAGE_WIDTH_IMX378   ((((FULL_IMAGE_WIDTH_IMX378 * POLYFIR_U_H_NUM_IMX378) - 1) / POLYFIR_U_H_DEN_IMX378) + 1)
#define POLY_UV_IMAGE_HEIGHT_IMX378  ((((FULL_IMAGE_HEIGHT_IMX378 * POLYFIR_U_V_NUM_IMX378) - 1) / POLYFIR_U_V_DEN_IMX378) + 1)

#define POLY_Y_IMAGE_WIDTH_IMX214    ((((FULL_IMAGE_WIDTH_IMX214 * POLYFIR_Y_H_NUM_IMX214) - 1) / POLYFIR_Y_H_DEN_IMX214) + 1)
#define POLY_Y_IMAGE_HEIGHT_IMX214   ((((FULL_IMAGE_HEIGHT_IMX214 * POLYFIR_Y_V_NUM_IMX214) - 1) / POLYFIR_Y_V_DEN_IMX214) + 1)
#define POLY_UV_IMAGE_WIDTH_IMX214   ((((FULL_IMAGE_WIDTH_IMX214 * POLYFIR_U_H_NUM_IMX214) - 1) / POLYFIR_U_H_DEN_IMX214) + 1)
#define POLY_UV_IMAGE_HEIGHT_IMX214  ((((FULL_IMAGE_HEIGHT_IMX214 * POLYFIR_U_V_NUM_IMX214) - 1) / POLYFIR_U_V_DEN_IMX214) + 1)

#define POLY_Y_IMAGE_WIDTH    ((((FULL_IMAGE_WIDTH * POLYFIR_Y_H_NUM) - 1) / POLYFIR_Y_H_DEN) + 1)
#define POLY_Y_IMAGE_HEIGHT   ((((FULL_IMAGE_HEIGHT * POLYFIR_Y_V_NUM) - 1) / POLYFIR_Y_V_DEN) + 1)
#define POLY_UV_IMAGE_WIDTH   ((((FULL_IMAGE_WIDTH * POLYFIR_U_H_NUM) - 1) / POLYFIR_U_H_DEN) + 1)
#define POLY_UV_IMAGE_HEIGHT  ((((FULL_IMAGE_HEIGHT * POLYFIR_U_V_NUM) - 1) / POLYFIR_U_V_DEN) + 1)


#define SIPP_HW_EDGE_OP_BUFFER_WIDTH      1296
#define SIPP_HW_EDGE_OP_BUFFER_HEIGHT     972
#define SIPP_HW_EDGE_OP_BUFFER_NUM_PLANES  1
#define SIPP_HW_EDGE_OP_BUFFER_SIZE  (SIPP_HW_EDGE_OP_BUFFER_WIDTH * \
              SIPP_HW_EDGE_OP_BUFFER_HEIGHT * SIPP_HW_EDGE_OP_BUFFER_NUM_PLANES)

// Application specific parameters
#define INPUT_MODE          NORMAL_MODE
#define OUTPUT_MODE         ORIENT_8BIT
#define THETA_MODE          NORMAL_THETA
#define THETA_OVX           ENABLED
#define MAGN_SCALE_FACT     0x3C00

void ISPPipeCreateParams();

#endif // _ISP_PIPE_SETTINGS_H_
