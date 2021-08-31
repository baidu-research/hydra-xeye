///
/// @file
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved
///            For License Warranty see: common/license.txt
///
/// @defgroup CameraModules Camera Modules
/// @defgroup CameraDefines Camera Defines
/// @ingroup CameraModules
/// @{
/// @brief Definitions and types needed by the Camera Module component
///
/// This is the file that contains all the Sensor definitions of constants, typedefs,
/// structures or enums or exported variables from this module

#ifndef _CAMERA_API_DEFINES_H_
#define _CAMERA_API_DEFINES_H_

#include "DrvMipiDefines.h"
#include "swcFrameTypes.h"

/// @brief I2C configuration descriptor
typedef struct{
	/// @brief Number of registers to be configured
   u32              numberOfRegs;
   /// @brief Delay in miliseconds to complete configuration
   u32              delayMs;
} I2CConfigDescriptor;

///  @brief MIPI configuration
typedef struct
{
	/// @brief Pixel format
   eDrvMipiDataType pixelFormat;
   /// @brief Data bitrate 
   u32              dataRateMbps;
   /// @brief Number of lanes
   u32              nbOflanes;
   /// @brief MIPI data transfer mode
   eDrvMipiDataMode dataMode;
} mipiSpec;                      //set all fields to 0 for parallel interface

/// @brief Configuration structure containing camera sensor dependent informations
typedef struct 
{
	/// @brief Frame width
    u32              frameWidth;
	/// @brief Frame height
    u32              frameHeight;
	/// @brief Horizontal back porch
    u32              hBackPorch;
	/// @brief Horizontal front porch
    u32              hFrontPorch;
	/// @brief Vertical back porch
    u32              vBackPorch;
	/// @brief Vertical front porch
    u32              vFrontPorch;
	/// @brief Pointer to MIPI configuration structure
    const mipiSpec   *mipiCfg;
	/// @brief Internal pixel format
    frameType        internalPixelFormat;
	/// @brief Bytes per pixel 
    u32              bytesPerPixel;
	/// @brief The ideal reference frequency
    u32              idealRefFreq;
	/// @brief I2C address for sensor 1
    u32              sensorI2CAddress1;
	/// @brief I2C address for sensor 2	
    u32              sensorI2CAddress2;
	/// @brief Number of I2C configuration steps
    u32              nbOfI2CConfigSteps;
	/// @brief Pointer to the struct containing descriptors for each step of I2C configuration
    I2CConfigDescriptor *i2cConfigSteps;
	/// @brief Register size in bytes
    u32              regSize;
	/// @brief Pointer to the two-dimentional array of I2C registers to be configured
    const u16       (*regValues)[2];
} GenericCamSpec;

/// @}
#endif // _CAMERA_API_DEFINES_H_
