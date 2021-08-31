ECHO ?= @

MV_SOC_PLATFORM  ?= myriad2
MV_SOC_REV       ?= ma2480
TEST_HW_PLATFORM := "MV0235_MA2x8x, MV0247_MA2085"
TEST_TYPE        := "AUTO"
EVM_BOARD_USED = MV0235
MV_MVDBG_GEN = 2
DISABLE_MVTENSOR_SOURCE = yes
SHAVE_COMPONENTS = yes
ROOT_BASE  ?= ../..
MV_COMMON_BASE  ?= $(ROOT_BASE)/xos/mdk/common
MV_TENSOR_BASE  ?= $(ROOT_BASE)/xos/mdk/projects/MvTensor
LEON_RT_BUILD   ?= yes

ComponentList_LOS   := LeonIPC PipePrint FatalExtension

ComponentList_LRT   += UnitTestVcs Fp16Convert VcsHooks LeonIPC PipePrint FatalExtension JpegM2EncoderDyn
ComponentList_SVE := JpegM2EncoderDyn
ComponentList_LOS += PreProcess
ComponentList_SVE += PreProcess
# include platform config
include $(ROOT_BASE)/xcore/platform/platform.mk
ComponentList_LOS += utils

DirAppRoot= .

MVSIMOPT        += -darw
DirAppOutput= ./output
MV_SOC_OS = rtems
RTEMS_BUILD_NAME = b-prebuilt

MV_USB_PROTOS = protovideo

# update for LCD ISR routing
CCOPT_LRT   += -DUse_LcdGeneric_on_LRT

CCOPT        += -I$(SIPP_DIR)
CCOPT        += -I$(SIPP_DIR)/include

#Sipp options
CCOPT       += -DSIPP_NO_PRINTF

MVASMOPT    += -a -noSlotAllPromo
#########################################################################################
# Include SIPP specifics
include $(MV_COMMON_BASE)/components/sipp/arch/$(MV_SOC_REV)/build/myriad2/sippMyriad2Elf.mk
#########################################################################################
CCOPT_LRT   += -D'SIPP_NO_PRINTF'
CCOPT_LRT   += -U'SIPP_CMX_POOL_SZ'
CCOPT_LRT   += -DSIPP_NO_PRINTF
CCOPT_LRT   += -DSIPP_CMX_POOL_SZ=196608
CCOPT_LRT   += -D'CMX_DATA=__attribute__((section(".cmx.data")))'
CCOPT_LRT   += -DDDR_SIPP_DATA

#Use asm code
MVCCOPT      += -D'SIPP_USE_MVCV'
MVCCOPT_LRT  += -D'SIPP_USE_MVCV'


CCOPT += -D'XEYE_DMA_WAIT_COUNT'
CCOPT_LRT += -D'XEYE_DMA_WAIT_COUNT'
MVCCOPT      += -D'XEYE_DMA_WAIT_COUNT'
MVCCOPT_LRT  += -D'XEYE_DMA_WAIT_COUNT'

MV_LOCAL_OBJ = no

MYRIAD = myriad2Asm
#The binary elf for Shave:
svuSippImg = $(DirAppOutput)/svuSippImg
RAWDATAOBJECTFILES += $(svuSippImg)_sym.o
RAWDATAOBJECTFILES += $(svuSippImg)Map.o

#Include Sipp Filter Paths
include ./build/sipp/*.mk

# Include the top makefile
PROJECTCLEAN += $(SippSvuObj)

include $(MV_TENSOR_BASE)/mvTensorSources.mk
include $(MV_COMMON_BASE)/generic.mk
include $(MV_TENSOR_BASE)/mvTensorRules.mk

#CCOPT += -DTESTIMAGE
#CCOPT_LRT += -DTESTIMAGE
CCOPT_LRT += -U'DEFAULT_HEAP_SIZE'
CCOPT_LRT += -D'DEFAULT_HEAP_SIZE=100960'
CCOPT_LRT += -D$(EVM_BOARD_USED)
CCOPT     += -D$(EVM_BOARD_USED)
CONLY_OPT += -std=gnu99
CPPONLY_OPT += -fpermissive
CCOPT               += -Wno-unused-variable -Wno-error -DFATHOMRUN
CCOPT_LRT           += -DFATHOMRUN
CCOPT_EXTRA_LRT     += -DFATHOMRUN
MVCCOPT             += -Wno-unused-variable -Wno-error -gdwarf-2 -ffast-math -fslp-vectorize
MVCCOPT_LRT         += -Wno-unused-variable -Wno-error -gdwarf-2 -ffast-math -fslp-vectorize
MVCCOPT_EXTRA_LRT   += -Wno-unused-variable -Wno-error -gdwarf-2 -ffast-math -fslp-vectorize
LEONOPTLEVEL        = -O3

#########################################################################################
# Add rules for SIPP binaries:
$(svuSippImg).mvlib : $(SippSvuObj)
	@mkdir -p $(dir $@)
	$(ECHO) $(LD) $(MVLIBOPT) $(SippSvuObj) -o $@

$(svuSippImg)_sym.o : $(svuSippImg).shvdcomplete
	$(ECHO) $(OBJCOPY) --prefix-symbols=lrt_SS_ --extract-symbol $< $@

$(DirAppObjBase)$(DirAppRoot)/leon_rt/appMemMap_lrt.o : $(svuSippImg).shvdlib
$(svuSippImg)Map.o : $(svuSippImg).shvdlib
		@mkdir -p $(dir $@)
			$(OBJCOPY) -I binary $(REVERSE_BYTES) --rename-section .data=.ddr.data \
					--redefine-sym  _binary_$(subst /,_,$(subst .,_,$<))_start=mbinImgSipp \
						-O elf32-littlesparc -B sparc $< $@

TEST_TAGS:= "MA2x8x, MA2085"
