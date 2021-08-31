CCOPT        += -I$(SIPP_DIR)
CCOPT        += -I$(SIPP_DIR)/include
#########################################################################################
#Sipp options
CCOPT       += -DSIPP_NO_PRINTF

MVASMOPT    += -a -noSlotAllPromo

#CCOPT += -DDISABLE_LEON_ICACHE -DDISABLE_LEON_DCACHE
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

MV_LOCAL_OBJ = no

MYRIAD = myriad2Asm
#The binary elf for Shave:
svuSippImg = $(DirAppOutput)/svuSippImg
RAWDATAOBJECTFILES += $(svuSippImg)_sym.o
RAWDATAOBJECTFILES += $(svuSippImg)Map.o

#Include Sipp Filter Paths
#include ./build/sipp/*.mk

# Include the top makefile
PROJECTCLEAN += $(SippSvuObj)




