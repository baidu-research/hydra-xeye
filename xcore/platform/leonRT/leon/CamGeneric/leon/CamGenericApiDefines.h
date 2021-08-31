///
/// @file
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved
///            For License Warranty see: common/license.txt
///
/// @defgroup CamGenericApiDefines CAM sensor API Defines
/// @ingroup CamGeneric
/// @{
/// @brief Definitions and types needed by the CamGeneric component
///
/// This file contains all the definitions of constants, typedefs,
/// structures, enums and exported variables for the generic camera component
///

#ifndef _CAM_GENERIC_API_DEFINES_H_
#define _CAM_GENERIC_API_DEFINES_H_

#include "swcFrameTypes.h"
#include "DrvI2cMasterDefines.h"
#include "DrvMipiDefines.h"  //?
#include "CameraDefines.h"


// 1: Defines
// ----------------------------------------------------------------------------

// 2: Typedefs (types, enums, structs)
// ----------------------------------------------------------------------------

/// @brief List of myriad receiver Id's(unified list) for a specific sensor
typedef enum
{
	CIF_DEVICE0 = 0,
	CIF_DEVICE1,
	SIPP_DEVICE0,
	SIPP_DEVICE1,
	SIPP_DEVICE2,
	SIPP_DEVICE3
}camRxId_type;

/// @brief Myriad receiver type for a specific sensor
typedef enum
{
	CIF_TYPE = 0,
	SIPP_TYPE
}camRxDevice_type;

/// @brief Status of the camera system
typedef enum
{
    /// @brief the camera system is running
    CAM_STATUS_ACTIVE = 0,             
    /// @brief The camera system is in standby, with the sensor in hot standby (configured but not streaming) and the myriad2 components in warm standby (mostly configured, less the MIPI PHY)	
    CAM_STATUS_HOT_STANDBY,        
	/// @brief The camera system is in standby, with the sensor in software standby (not configured) and the myriad2 components in warm standby (mostly configured, less the MIPI PHY). Final reactivity of the system in this status is "cold" (slow) , even if the myriad2 is "active", because the sensor configuration last a huge time	
    CAM_STATUS_COLD_STANDBY                                
}camStatus_type;

/// @brief List of returned errors by CamGeneric API
typedef enum
{
	/// @brief Operation performed successfully
   CAM_SUCCESS = 0,   
	/// @brief The list of parameters sent to the current CamGeneric function contains NULL values for mandatory parameters   
   CAM_ERR_PARAMETERS_LIST_INCOMPLETE,                      
	/// @brief  The list of parameters sent to the current CamGeneric function contains conflicting buffer allocation functions
   CAM_ERR_CONFLICTING_ALLOC_FUNCTIONS,                   
   /// @brief The id of the sensor receiver inside myriad is not correct
   CAM_ERR_RECEIVER_ID_INCORRECT,                         
	/// @brief Neither an I2C handle, nor an I2C configuration callback function were previously set for the current camera, so the sensor (re)configuration can not be performed   
   CAM_ERR_MISSING_CALLBACK_OR_I2C_HANDLE,        
	/// @brief An I2C communication error with the sensor happened   
   CAM_ERR_I2C_COMMUNICATION_ERROR,   
	/// @brief The parameters used for MIPI configuration were not correct
   CAM_ERR_MIPI_CONFIGURATION_ERROR,                  
	/// @brief An error happened while initializing the MIPI: the PLL frequency could not be stabilized   
   CAM_ERR_MIPI_PLL_INITIALISATION_ERROR,                    
    /// @brief The configured dynamic IRQ, set in routeInterrupt parameter, is not correct
   CAM_ERR_ROUTED_INTERRUPT_NOT_VALID,          
	/// @brief The requested or current status of the camera does not permit the requested action   
   CAM_ERR_STATUS_NOT_APPROPRIATE,                  
   /// @brief This feature is currently not implemented, but is foreseen to be in the next releases   
   CAM_ERR_FEATURE_TEMPORARY_NOT_IMPLEMENTED,             
   /// @brief This feature is not available for the requested HW configuration
   CAM_ERR_FEATURE_NOT_AVAILABLE_FOR_CURRENT_CONFIGURATION   
}camErrorType;

/// @brief List of configurable interrupt types
typedef enum
{
	/// @brief No interrupt
	NO_CAM_ISR               = 0x0,         
	/// @brief CIF end of line
    CIF_INT_EOL              = 0x00000001,  
	/// @brief CIF end of frame
	CIF_INT_EOF              = 0x00000002,  
	/// @brief CIF DMA0 done
	CIF_INT_DMA0_DONE        = 0x00000004,  
	/// @brief CIF DMA0 overflow
    CIF_INT_DMA0_OVERFLOW    = 0x00000010,  
	/// @brief CIF DMA0 underrun
    CIF_INT_DMA0_UNDERFLOW   = 0x00000020,  
	/// @brief CIF DMA0 FIFO full
    CIF_INT_DMA0_FIFO_FULL   = 0x00000040,  
	/// @brief CIF DMA0 FIFO empty
    CIF_INT_DMA0_FIFO_EMPTY  = 0x00000080, 
	/// @brief CIF DMA1 done
    CIF_INT_DMA1_DONE        = 0x00000100,
	/// @brief CIF DMA1 idle
    CIF_INT_DMA1_IDLE        = 0x00000200,
	/// @brief CIF DMA1 overflow
    CIF_INT_DMA1_OVERFLOW    = 0x00000400,
	/// @brief CIF DMA1 underflow
    CIF_INT_DMA1_UNDERFLOW   = 0x00000800,
	/// @brief CIF DMA1 FIFO full
    CIF_INT_DMA1_FIFO_FULL   = 0x00001000,
	/// @brief CIF DMA1 FIFO empty
    CIF_INT_DMA1_FIFO_EMPTY  = 0x00002000,
	/// @brief CIF DMA1 done
    CIF_INT_DMA2_DONE        = 0x00004000,
	/// @brief CIF DMA1 idle
    CIF_INT_DMA2_IDLE        = 0x00008000,
	/// @brief CIF DMA2 overflow
    CIF_INT_DMA2_OVERFLOW    = 0x00010000,
	/// @brief CIF DMA2 underflow
    CIF_INT_DMA2_UNDERFLOW   = 0x00020000,
	/// @brief CIF DMA2 FIFO full
    CIF_INT_DMA2_FIFO_FULL   = 0x00040000,
	/// @brief CIF DMA2 FIFO empty
    CIF_INT_DMA2_FIFO_EMPTY  = 0x00080000,
	/// @brief SIPP DMA done for line events; value not correlated to the HW
	SIPP_INT_LINE_DMA_DONE   = 0x00100000, 
	/// @brief SIPP DMA done for frame events; value not correlated to the HW
	SIPP_INT_FRAME_DMA_DONE  = 0x00200000, 
	/// @brief SIPP exceptional event : one (or more) frame is lost; value not correlated to the HW
	SIPP_INT_FRAME_LOST      = 0x00400000  
}camIsrType;

/// @brief List of receiver re-synchronization types (CIF/SIPP resync with MIPI)
typedef enum
{
    /// @brief No resynchronization between CIF and MIPI
    NO_RESYNC            = 0x0,
    /// @brief The CIF DMA window is reseted once at the time of the first EndOfFrame event in MIPI controler
    RESYNC_ONCE,
    /// @brief The CIF DMA window is reseted on reception of every EndOfFrame event in MIPI controler
    RESYNC_CONTINUOUSLY
}externalResyncType;

/// @brief Synchronization signals for sensors connected on parallel interface
typedef struct
{
	/// @brief Vertical synchronization signal for GPIO
    u8 vSyncGpio;
    u8 vSyncGpioMode;
	/// @brief Horizontal synchronization signal for GPIO
    u8 hSyncGpio;
    u8 hSyncGpioMode;
}camSyncSignalsType;

/// @brief Configuration structure containing application dependent informations
typedef struct
{
	/// @brief The id of MIPI controller the sensor is connected to
   eDrvMipiCtrlNo        mipiControllerNb;   
   /// @brief The unified id of the receiver camera block
   camRxId_type          receiverId;         
   /// @brief The GPIO id used to reset the sensor, or value 0xFF if no reset is to be performed by the CamGeneric
   u32                   sensorResetPin;     
   /// @brief The id of the I2C address inside the sensor header (see mipiCam component): 0 for the first address, 1 if the second one is used by CamGeneric
   u32                   stereoPairIndex;    
   /// @brief Vertical start position of the cropping window for the received sensor image
   u32                   windowRowStart;     
   /// @brief Horizontal start position of the cropping window for the received sensor image
   u32                   windowColumnStart;  
   /// @brief width of the cropping window for the received sensor image
   u32                   windowWidth;        
   /// @brief Height of the cropping window for the received sensor image
   u32                   windowHeight;       
   /// @brief Structure containing all the info needed to generate the synchronization signals for sensors connected on parallel interface (not MIPI)
   camSyncSignalsType   *generateSync;      
} CamUserSpec;

/// @brief List of user callback functions (cbf) to be called by CamGeneric on specific events
typedef struct
{
	/// @brief cbf called once by CamStart + on every ISR at the end of frame data transfer, in order to provide a pointer to the next frame buffer(and associated structure) to be filled by CamGeneric;
	/// @note optional parameter (see getBlock also)
   frameBuffer*  (*getFrame)(void);              

	/// @brief cbf called once by CamStart + on every ISR at the end of line data transfer, in order to provide a pointer to the next line buffer to be filled by CamGeneric; optional parameter.
	/// @note only one (at maximum) of getFrame / getBlock function must exist and will be called by the CamGeneric;
   frameBuffer*  (*getBlock)(u32);               

	/// @brief cbf called by CamGeneric ISR on any of the ISRs set in notificationInterrupts, to provide additional user functionalities
	/// @note optional parameter
   void          (*notification)(u32);           
}interruptsCallbacksListType;


/// @brief List of user callback functions (cbf) to be called by CamGeneric while configuring the external video sensor
typedef struct
{
	/// @brief cbf to provide custom initialization of the sensor; MANDATORY if no I2C handler was provided in CamInit
   camErrorType  (*sensorPowerUp)(I2CM_Device*, GenericCamSpec*, CamUserSpec*);     
    /// @brief cbf to provide custom standby sequence of the sensor; MANDATORY if no I2C handler was provided in CamInit
   camErrorType  (*sensorStandby)(I2CM_Device*, GenericCamSpec*, CamUserSpec*);   
	/// @brief cbf to provide custom wakeup sequence of the sensor; MANDATORY if no I2C handler was provided in CamInit
   camErrorType  (*sensorWakeup)(I2CM_Device*, GenericCamSpec*, CamUserSpec*);      
   /// @brief cbf to provide custom power down sequence of the sensor; MANDATORY if no I2C handler was provided in CamInit
   camErrorType  (*sensorPowerDown)(I2CM_Device*, GenericCamSpec*, CamUserSpec*);   
}sensorCallbacksListType;

/// @brief List of sensor configuration and called on interrupt lists of callback functions (cbf)
typedef struct
{
	/// @brief List of external sensor configuration cbf
    sensorCallbacksListType     *sensorCbfList;
	/// @brief List of cbf called on interrupts
    interruptsCallbacksListType *isrCbfList;
}callbacksListStruct;


/// @brief CamGeneric private structure for every camera handle; allocated in the application space but updated by CamGeneric API; MANDATORY parameter in any API call
typedef struct
{
	/// @brief Type of receiver ensor 
    camRxDevice_type             receiverType;
	/// @brief The unified id of the receiver camera block
    camRxId_type                 receiverId;
	/// @brief CIF type receiver Id
    u32                          cifId;
	/// @brief SIPP type receiver Id
    u32                          sippId;
	/// @brief  frame counter
    volatile u32                 frameCount;
	/// @brief Status of the camera system
    camStatus_type               status;
	/// @brief I2C device handle
    I2CM_Device*                 pI2cHandle;
	/// @brief Configuration structure containing camera sensor dependent informations
    GenericCamSpec               camSpec;
	/// @brief Configuration structure containing application dependent informations
    CamUserSpec                  usrSpec;
	/// @brief Pointer to the fully received frame buffer
    frameBuffer*                 receivedFrame;
    /// @brief Pointer to the current frame buffer to receive
    frameBuffer*                 runningFrame;
    /// @brief Pointer to the next frame buffer to receive
    frameBuffer*                 nextFrame;
    /// @brief Interrupt priority
    u32                          interruptsPriority;
	/// @brief List of cbf called on interrupts
    interruptsCallbacksListType  isrFPtr;
	/// @brief  List of external sensor configuration cbf
    sensorCallbacksListType      sensorFPtr;
}GenericCameraHandle;


// 3: Local types (static configuration types)
// ----------------------------------------------------------------------------

/// @brief Registers and masks for CIF
typedef struct
{
	/// @brief CIF base address
    const unsigned int     CIF_BASE_ADR;
	/// @brief Interrupt clear register address
    const unsigned int     INT_CLEAR_ADR;
	/// @brief Interrupt enable register address
    const unsigned int     INT_ENABLE_ADR;
	/// @Interrupt status register address
    const unsigned int     INT_STATUS_ADR;
	/// @brief Line count register address
    const unsigned int     LINE_COUNT_ADR;
	/// @brief Frame count register address
    const unsigned int     FRAME_COUNT_ADR;
	/// @brief Default interrupt id 
    const unsigned int     int_id_default;
	/// @brief Interrupt clear mask
    volatile unsigned int  int_clear_mask;
	/// @brief Interrupt id
    volatile unsigned int  int_id;
	/// @brief Interrupt management mask
    volatile unsigned int  int_management_mask;
	/// @brief Interrupt notification mask
    volatile unsigned int  int_notification_mask;
    /// @brief CIF DMA synchronization with MIPI
    volatile externalResyncType  MIPI_sync;
} CifHwRegsAndMasks;

/// @brief Registers and masks for SIPP
typedef struct
{
	/// @brief CIF base address
    const unsigned int     SIPP_BASE_ADR;
	/// @brief Line count register address
    const unsigned int     LINE_COUNT_ADR;
	/// @brief Default line interrupt id
    const unsigned int     line_int_id_default;
	/// @brief Default frame interrupt id
    const unsigned int     frame_int_id_default;
	/// @brief Default interrupt mask
    const unsigned int     int_default_mask;
	/// @brief Interrupt clear line mask
    volatile unsigned int  int_clear_line_mask;
	/// @brief Interrupt clear frame mask
    volatile unsigned int  int_clear_frame_mask;
	/// @brief Line interrupt id
    volatile unsigned int  line_int_id;
	/// @brief Frame interrupt id
    volatile unsigned int  frame_int_id;
	/// @brief Interrupt management line mask
    volatile unsigned int  int_management_line_mask;
	/// @brief Interrupt management frame mask
    volatile unsigned int  int_management_frame_mask;
	/// @brief Interrupt notification line mask
    volatile unsigned int  int_notification_line_mask;
	/// @brief Interrupt notification frame mask
    volatile unsigned int  int_notification_frame_mask;
} SippHwRegsAndMasks;

/// @}
#endif //_CAM_GENERIC_API_DEFINES_H_
