///
/// @file
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved
///            For License Warranty see: common/license.txt
///
/// @defgroup CamGeneric Camera Generic
/// @defgroup CamGenericApi CAM sensor API
/// @ingroup  CamGeneric
/// @{
/// @brief    Generic Camera API.
///
/// This is the API for generic camera component
///


#ifndef _CAM_GENERIC_API_H_
#define _CAM_GENERIC_API_H_

// 1: Includes
// ----------------------------------------------------------------------------
#include "CamGenericApiDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

// 2:  Exported Global Data (generally better to avoid)
// ----------------------------------------------------------------------------
// 3:  Exported Functions (non-inline)
// ----------------------------------------------------------------------------

/// @brief This will initialize the camera component (the sensor and all the myriad components connected in the frames data path) for a specific sensor
/// @param[in]  hndl         - Pointer to the empty camera handle which will be filled back with all camera configurations at return (pseudo return parameter)
/// @param[in]  camSpec      - Fixed camera configuration
/// @param[in]  userSpec     - Dynamic camera configuration
/// @param[in]  cbStruct     - structured list of pointers to all user callback functions which will be called by CamGeneric
/// @param[in]  pI2cHandle   - I2C handle, NULL if no default I2C configuration need to be performed by CamGeneric
/// @return camErrorType - CAM_SUCCESS if camera initialization was OK, other values if it failed
///
camErrorType CamInit(GenericCameraHandle *hndl, GenericCamSpec *camSpec, CamUserSpec *userSpec, callbacksListStruct* cbStruct, I2CM_Device *pI2cHandle);


/// @brief This will start the sensor and the interrupts system
/// @param[in]  hndl         - Pointer to the camera handle
/// @return camErrorType - CAM_SUCCESS if camera started OK, other values if it failed
///
camErrorType CamStart(GenericCameraHandle *hndl);


/// @brief This will put the sensor and related myriad logic in a standby mode
/// @param[in]  hndl        - Pointer to the camera handle
/// @param[in]  standbyType - The standby type to be performed: COLD or HOT
/// @return camErrorType  - CAM_SUCCESS if camera standby passing was OK, other values if it failed
///
camErrorType CamStandby(GenericCameraHandle *hndl, camStatus_type standbyType);


/// @brief This will wake up the sensor and related myriad logic from the standby mode
/// @param[in]  hndl         - Pointer to the camera handle
/// @return camErrorType - CAM_SUCCESS if camera wake up was OK, other values if it failed
///
camErrorType CamWakeup(GenericCameraHandle *hndl);


/// @brief Resets the camera component (the sensor and the related myriad logic)
/// @param[in]  hndl         - Pointer to the camera handle
/// @return camErrorType - CAM_SUCCESS if camera was stopped OK, other values if it failed
///
camErrorType CamStop (GenericCameraHandle *hndl);


/// @brief Set or change the existing callbacks used for the sensor configuration.
/// @param[in]  hndl         - Pointer to the camera handle
/// @param[in]  cbList       - List of callbacks pointers
/// @return camErrorType - CAM_SUCCESS if the camera callbacks list was updated OK, other values if it failed
///
camErrorType CamSetupCallbacks(GenericCameraHandle* hndl, sensorCallbacksListType* cbList);


/// @brief Set or change the existing/the default interrupt events for a camera module.
/// @param[in] hndl 		       - Pointer to the camera handle
/// @param[in] managedInterrupt    - The event on which CamGeneric will perform local HW management (DMA restart, ISRs clearing) (values: see camIsrType) and will call the allocation cbf
/// @param[in] notifiedInterrupts  - The event(s) on which CamGeneric will call the notification cbf (values: combined values of camIsrType type)
/// @param[in] clearedInterrupts   - The event(s) which will be cleared locally in in CamGeneric ISR handler (values: combined values of camIsrType type)
/// @param[in] cbList 			   - The list of callbacks to be used by the ISR events
/// @param[in] interruptsLevel     - The level of priority to be assigned to the camera interrupt; values between 0 - 15 (not recommended)
/// @param[in] routeLineInterrupt  - The id of the rerouted line interrupt, in case CamGeneric runs on Leon OS (values: 0 = default, IRQ_DYNAMIC_0 .. IRQ_DYNAMIC_11, seeDrvIcbDefines.h)
///              		             For CIF receivers, this must have same value as the routeFrameInterrupt
/// @param[in] routeFrameInterrupt - The id of the rerouted frame interrupt, in case CamGeneric runs on Leon OS (values: 0 = default, IRQ_DYNAMIC_0 .. IRQ_DYNAMIC_11, seeDrvIcbDefines.h)
/// @return camErrorType            CAM_SUCCESS if the camera interrupts registers were updated OK, other values if it failed
///
camErrorType CamSetupInterrupts(GenericCameraHandle* hndl, camIsrType managedInterrupt, u32 notifiedInterrupts, u32 clearedInterrupts,
                                 interruptsCallbacksListType* cbList, u32 interruptsLevel, u32 routeLineInterrupt, u32 routeFrameInterrupt );


/// @brief Get the counter of frames/lines (depending on the managed ISR types) received inside CIF block of myriad component (always 0 for SIPP receivers)
/// @param  hndl - Pointer to the camera handle
/// @return Frame number
///
unsigned int CamGetFrameCounter (GenericCameraHandle *hndl);


/// @}
#ifdef __cplusplus
}
#endif


#endif //_CAM_GENERIC_API_H_
