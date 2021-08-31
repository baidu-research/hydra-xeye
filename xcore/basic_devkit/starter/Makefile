# ------------------------------[ App ]--------------------------------------------#
# Default application name is the same as the folder it is in.
# This can be overridden here if something different is required
#APPNAME ?= $(notdir $(shell pwd))

# Verbose or not, common out for more verbosity
#ECHO ?=@
#ECHO =
#RUN_JUST_ON_LOS    ?=yes

# ------------------------------[ Build overrides ]--------------------------------#
JENKINS_HOME ?= ""
GZZ_APP_CONFIG  ?=master_all
MV_SOC_PLATFORM ?= myriad2
MV_SOC_REV      ?= ma2480
MAKEDEPEND      ?= no
MV_TOOLS_DIR    = ../../../xos/tools
#DISABLE_ERROR_ON_WARNINGS=yes
# Ensure that the we are using the correct rtems libs etc
RTEMS_BUILD_NAME = b-prebuilt
MV_SOC_OS     = rtems
ifeq ($(RUN_JUST_ON_LOS),yes)
else
MV_SOC_OS_LRT = rtems
LEON_RT_BUILD = yes
endif
BUILD_FOR_TUNINGTOOL ?= no
RUN_POWER_MEASUREMENT ?=yes

# Set MV_COMMON_BASE relative to mdk directory location (but allow user to override in environment)
MDK_ROOT =  ../../../xos/mdk
MV_COMMON_BASE  ?= $(MDK_ROOT)/common
AllLibs  += ./lib/SunnyLib.a

GUZZI_INC_DIR =  $(realpath $(MDK_ROOT)/packages/movidius/Guzzi/arch/ma2x8x/include)
GUZZI_LIB_DIR =  $(realpath $(MDK_ROOT)/packages/movidius/Guzzi/arch/ma2x8x/lib)
output/$(APPNAME).elf: $(GUZZI_LIB_DIR)/libguzzi_ma2x8x.a

ComponentList_LOS += XLink
ComponentList_LOS += USB_VSC

ComponentList_LOS += flic/plugins/arch/ma2x8x/plgXlink
ComponentList_LOS += PreProcess
ComponentList_SVE += PreProcess
MVCCOPT += -D'MVCV_USE_NEW_BILINEAR'
MVCCASMOPT += -D'MVCV_USE_NEW_BILINEAR'
MVCCOPT += -D'SRC_MAX_WIDTH=4056'
MVCCOPT += -D'DEST_MAX_WIDTH=300'
CCOPT += -D'USB_STANDALONE_MEM_POOL'
CCOPT += -D'MVNCI_USE_FIXED_RESOURCES'
CCOPT += -D'MVTENSOR_PREPROC_REUSE_SHAVES'
CCOPT += -D'SHAVES_ALWAYS_POWER_ON'

#Component lists:
ComponentList += VcsHooks Fp16Convert PipePrint FatalExtension
ComponentList += flic/components/logIspL
ComponentList += flic/common/commonTypes
ComponentList += flic/core
ComponentList += flic/types
ComponentList += flic/plugins/arch/ma2x8x/common/commonTypes
ComponentList += flic/plugins/arch/ma2x8x/common/ispCommon
ComponentList += flic/plugins/arch/ma2x8x/plgSrcMipi
ComponentList += flic/plugins/arch/ma2x8x/plgSrcCtrl
ComponentList += flic/plugins/arch/ma2x8x/plgEventsRec
ComponentList += flic/plugins/arch/ma2x8x/plgColorIsp
ComponentList += flic/plugins/arch/ma2x8x/plgIspCtrl
ComponentList += flic/plugins/arch/ma2x8x/plgIspStilCtrl
ComponentList += flic/plugins/arch/ma2x8x/plgIspStilItf
ComponentList += flic/pipes/arch/ma2x8x/common
ifeq ($(RUN_JUST_ON_LOS),yes)
ComponentList += flic/pipes/arch/ma2x8x/grpIspVdoZsl1L
else
ComponentList += flic/pipes/arch/ma2x8x/grpIspColorVdoZsl2L
endif
ComponentList += sipp/arch/ma2x8x/pipes

ComponentList_LOS += mvLibCamera
ComponentList_LOS += flic/components/arch/ma2x8x/camCtrlItf
ifeq ($(RUN_POWER_MEASUREMENT),yes)
ComponentList_LOS += MV0235
endif

ifeq ($(RUN_JUST_ON_LOS),yes)
else
UNIQUE_SYMBOLS += grpIspColorVdoZsl2L_OS grpIspColorVdoZsl2L_RT
COMM_DEFS += -D'FLIC_2LEONS'
COMM_DEFS += -D'DYN_IRQ_ASSIGNMENT=1'
endif
COMM_DEFS += -D'PLL_DESIRED_FREQ_KHZ=700000'
COMM_DEFS += -D'DEFAULT_OSC0_KHZ=24000'
COMM_DEFS += -D'FRAME_TIME=33'
COMM_DEFS += -D'PLG_ISP_MAX_W=4096'
COMM_DEFS += -D'PLG_ISP_MAX_H=3040'
COMM_DEFS += -D'MIPIRX_DISABLE_WAIT_LPSTATUS_IN=1'
#COMM_DEFS += -D'LOGISPL_ENABLE'
#COMM_DEFS += -D'USE_CUSTOM_GET_TIME'
#COMM_DEFS += -D'TASK_SWTCH_LOG_ENABLE'
#COMM_DEFS += -D'LOGSRCMIPI_ENABLE'
#COMM_DEFS += -D'LOGCOLORISP_ENABLE'
#COMM_DEFS += -D'LOGMONOISP_ENABLE'

CCOPT     += $(COMM_DEFS)
CCOPT_LRT += $(COMM_DEFS)

CCOPT     += -D'N_POOL_FRMS_RAW=8'
CCOPT     += -D'N_POOL_FRMS_VDO=5'
CCOPT     += -D'N_POOL_FRMS_STL=0'

CCOPT_LRT += -D'PRINT_PERFORMANCE'
CCOPT += -D'PRINT_PERFORMANCE'

ifeq ($(RUN_JUST_ON_LOS),yes)
CCOPT += -D'DEF_POOL_SZ = (6*2*4056*3040)+(3*2*4056*3040)+(2*2*4056*3040)+(3*2*1280*800)+(3*2*1280*800)'
CCOPT += -D'MAX_ZSL_BUF_SIZE=2'
CCOPT += -D'USE_ONE_LEON'
CCOPT += -D'SIPP_IRQ_DMA_PRIORITY=14'
else
CCOPT     += -D'DEF_POOL_SZ = 1024 + 4*(1024 + 1024*2*20) + 4*(1024 + 1024*2*20)'
CCOPT_LRT += -D'DEF_POOL_SZ = (8*2*2688*1520)+(5*2*2688*1520)'
CCOPT_LRT += -D'MAX_ZSL_BUF_SIZE=2'
CCOPT += -D'MAX_ZSL_BUF_SIZE=2'
CCOPT_LRT += -D'SIPP_IRQ_DMA_PRIORITY=14'
CCOPT += -D'SIPP_IRQ_DMA_PRIORITY=14'
endif

# CCOPT += -D'__TMS_CLOAKED_NAMES_ONLY'

#########################################################################################
# Include SIPP specifics
include $(MV_COMMON_BASE)/components/sipp/arch/$(MV_SOC_REV)/build/myriad2/sippMyriad2Elf.mk
#########################################################################################

# Use this makefile variable to specify the USB device class protos to be included into the build
# Available USB device protos: protohid protomsc protondfu protovideo protovsc2
USE_USB_VSC?=yes
ifeq ($(USE_USB_VSC),yes)
LEON_APP_URC_SOURCES +=  $(wildcard $(MV_COMMON_BASE)/components/USB_VSC/*leon/*.urc)
CCOPT+= -D'USE_USB_VSC'
ComponentList_LOS+=USB_VSC
MV_USB_PROTOS = protovsc2
else
LEON_APP_URC_SOURCES +=  $(wildcard $(MV_COMMON_BASE)/components/USB_CDC/*leon/*.urc)
MV_USB_PROTOS = protowmc
endif

ifeq ($(RUN_JUST_ON_LOS),yes)
CCOPT += -U SIPP_CMX_POOL_SZ -DSIPP_CMX_POOL_SZ=11000
else
CCOPT_LRT += -U SIPP_CMX_POOL_SZ -DSIPP_CMX_POOL_SZ=11000
CCOPT_LRT += -U CMX_DATA
CCOPT_LRT += -DCMX_DATA='__attribute__((section(".cmx.data")))'
endif
#CCOPT += -DSIPP_DDR_DATA_DEFAULT -DSIPP_DDR_COHERENCY_WORKAROUND
#CCOPT += -DSIPP_AMC_HIPRIO_AT_CMX
##CCOPT_LRT += -DSIPP_AMC_HIPRIO_AT_CMX
#CCOPT += -DSIPP_DDR_DATA_DEFAULT
#CCOPT += -DSIPP_NO_PRINTF -DNO_PRINT
#CCOPT += -DSIPP_NO_SHAVES_IN_BUILD
CCOPT += -DSIPP_USE_FREE_RT
CCOPT_LRT += -DSIPP_USE_FREE_RT

CCOPT += -DSIPP_WORKAROUND_ALL_DYNAMIC_IRQ_USE
CCOPT_LRT += -DSIPP_WORKAROUND_ALL_DYNAMIC_IRQ_USE

# --- LibCamera
#CCOPT += -D'MVLIBCAMERA_DEBUG'
#CCOPT += -D'OSDRVIMX378_DEBUG'
#CCOPT += -D'OSDRVVCM_DEBUG'
#CCOPT += -D'OSDRVEEPROM_DEBUG'
ifeq ($(RUN_POWER_MEASUREMENT),yes)
CCOPT += -D'POWER_MEASUREMENT'
endif

# ------------------------------[ Tools ]------------------------------------------#
# Hardcode tool version here if needed, otherwise defaults to Latest

#The binary elf for Shave:
svuSippImg = $(DirAppOutput)/svuSippImg
RAWDATAOBJECTFILES += $(svuSippImg)_sym.o
RAWDATAOBJECTFILES += $(svuSippImg)Map.o


PROJECTCLEAN += $(SippSvuObj)
# Include the generic Makefile, already included in mvTensorRuls.mk
#include $(MV_COMMON_BASE)/generic.mk

# MvTensor settings
MVTENSOR_DMA = OS_X
CCOPT += -D'MA2480'

LEON_HEADERS += $(wildcard $(MV_FATHOM_RUN_DIR)/leon/*.h)
LEON_COMPONENT_HEADERS_PATHS += $(MV_FATHOM_RUN_DIR)/leon
LEON_COMPONENT_CPP_SOURCES_LOS += $(wildcard $(MV_FATHOM_RUN_DIR)/leon/*.cpp)

# ------------------------------[ FathomRun ]--------------------------------#
LEON_HEADERS += $(MV_FATHOMRUN_LEON_HEADERS)
LEON_COMPONENT_HEADERS_PATHS += $(MV_FATHOMRUN_LEON_COMPONENT_HEADERS_PATHS)
LEON_COMPONENT_CPP_SOURCES_LOS += $(MV_FATHOMRUN_LEON_COMPONENT_CPP_SOURCES)
LEON_COMPONENT_PATHS_LOS += $(MV_FATHOMRUN_LEON_COMPONENT_PATHS)

SH_COMPONENTS_HEADERS_PATHS += $(MV_FATHOMRUN_SH_COMPONENTS_HEADERS_PATHS)

MV_FATHOMRUN_DEFINES += -DMVNCI_DISABLE_VERSION_CHECK
include $(MDK_ROOT)/projects/FathomRun/fathomRun.mk

CCOPT += $(MV_FATHOMRUN_DEFINES)
CPPOPT += $(MV_FATHOMRUN_DEFINES)

# -------------------------------- [ Build Options ] ------------------------------ #
LEONOPTLEVEL= -O3
MVCCOPT    += -ffast-math -fslp-vectorize
# ----------------------------------------------------------------------------- #

######### Extra app related options ##########
CCOPT			+= -DDEBUG
#CCOPT += -DNO_PRINT

CCOPT +=-DDEFAULT_ALLOC_TIMEOUT_MS=4000

CCOPT += -D__MMS_DEBUG__
CCOPT += -D___RTEMS___
CCOPT += -DSPI_SLAVE
CCOPT += -DCAMERA_INTERFACE_ANDROID_CAMERA3
CCOPT += -I$(GUZZI_INC_DIR)/osal/include
CCOPT += -I$(GUZZI_INC_DIR)/hif/rpc/include
CCOPT += -I$(GUZZI_INC_DIR)/hif/camera3/c/include
CCOPT += -I$(GUZZI_INC_DIR)/dtp/server/source/include
CCOPT += -I$(GUZZI_INC_DIR)/guzzi/modules
CCOPT += -I$(GUZZI_INC_DIR)/guzzi/modules/camera
CCOPT += -I$(GUZZI_INC_DIR)/guzzi/modules/libs
CCOPT += -I$(GUZZI_INC_DIR)/guzzi/modules/include
CCOPT += -I$(GUZZI_INC_DIR)/guzzi/modules/hal/include
CCOPT += -I$(GUZZI_INC_DIR)/guzzi/modules/version_info/include
CCOPT += -I$(GUZZI_INC_DIR)/algorithms/source/include/aca
CCOPT += -I$(GUZZI_INC_DIR)/algorithms/source/include/sg/algs
CCOPT += -I$(GUZZI_LIB_DIR)/dtp_db

CCOPT += -D'GZZ_AEWB_MERGER_SWITCH=0'

ifeq ($(GZZ_APP_CONFIG),master_all)
CCOPT += -D'GZZ_MASTER_SLAVE_SELECT=0'
endif
ifeq ($(GZZ_APP_CONFIG),single_master_slave)
CCOPT += -D'GZZ_MASTER_SLAVE_SELECT=2'
endif
ifeq ($(GZZ_APP_CONFIG),master_slave_slave)
CCOPT += -D'GZZ_MASTER_SLAVE_SELECT=1'
endif
ifeq ($(GZZ_APP_CONFIG),master_slave_alt)
CCOPT += -D'GZZ_MASTER_SLAVE_SELECT=3'
endif

CCOPT_LRT += -I$(IPIPE_BASE)/components/mod_i2c_command/leon/
CCOPT_LRT += -I$(MV_COMMON_BASE)/components/I2CSlave/leon

#CCOPT += -D'DTP_DB_NAME="database_OS05A10_2560x1440.bin"'

ifeq ($(BUILD_FOR_TUNINGTOOL),no)
CCOPT += -D'DTP_DB_NAME="database_IMX378_OV9282mono.bin"'
endif
ifeq ($(BUILD_FOR_TUNINGTOOL),yes)
CCOPT += -D'DTP_DB_NAME="database_IMX378_OV9282mono_4tt.bin"'
endif

DefaultSparcRTEMSLibs += $(GUZZI_LIB_DIR)/libguzzi_ma2x8x.a
DefaultSparcRTEMSLibs += $(GUZZI_LIB_DIR)/libversion_info.a

#########################################################################################
$(svuSippImg).mvlib : $(SippSvuObj) $(PROJECT_SHAVE_LIBS)
	$(ECHO) $(LD) $(MVLIBOPT) $(SippSvuObj) $(PROJECT_SHAVE_LIBS) $(CompilerANSILibs) -o $@

ifeq ($(RUN_JUST_ON_LOS),yes)
$(svuSippImg)_sym.o : $(svuSippImg).shvdcomplete
	$(ECHO) $(OBJCOPY) --prefix-symbols=SS_ --extract-symbol $< $@
else
$(svuSippImg)_sym.o : $(svuSippImg).shvdcomplete
	$(ECHO) $(OBJCOPY) --prefix-symbols=lrt_SS_ --extract-symbol $< $@
endif

$(svuSippImg)Map.o : $(svuSippImg).shvdlib
		@mkdir -p $(dir $@)
			$(ECHO) $(OBJCOPY) -I binary $(REVERSE_BYTES) --rename-section .data=.cmx.data \
					--redefine-sym  _binary_$(subst -,_,$(subst /,_,$(subst .,_,$<)))_start=mbinImgSipp \
						-O elf32-littlesparc -B sparc $< $@

TEST_TYPE   := "MANUAL"
TEST_TAGS   := "MA2480"
TEST_HW_PLATFORM = "MV0235_MA2480"
