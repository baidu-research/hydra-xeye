APPNAME ?= $(notdir $(shell pwd))
$(info "xeye platform detected APP name: $(APPNAME)")

RES_WIDTH ?= 1920
RES_HEIGHT ?= 1080
MAX_GRAPH ?= 2
# default relative directory to app
XEYE_PLATFORM= $(realpath --relative-to=${PWD} $(ROOT_BASE))/xcore/platform
XEYE_PLATFORM_PATH ?= $(XEYE_PLATFORM)

# add auto version compile
$(info $(shell cd $(XEYE_PLATFORM_PATH); bash ./mkcompile_h.sh))

# add leonRT common component
ComponentList_LRT += $(XEYE_PLATFORM)/leonRT
ComponentList_LRT += $(XEYE_PLATFORM)/leonShare

# # Available USB device protos: protohid protomsc protondfu protovideo protovsc2
MV_USB_PROTOS = protovsc2
# # Uncomment the following line to use the debug build of USB libraries
#RTEMS_USB_LIB_BUILD_TYPE = debug

# add leonOS common component
ComponentList_LOS += $(XEYE_PLATFORM)/leonOS
ComponentList_LOS += $(XEYE_PLATFORM)/leonShare

APPNAME_CFG=$(APPNAME)_cfg.mk
$(info "xeye platform detected APP name CFG File: $(APPNAME_CFG)")
include $(XEYE_PLATFORM)/config/$(APPNAME_CFG)

ifeq ($(CONFIG_SIPP_RGB),lrt)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/sipp/rgb
    include $(XEYE_PLATFORM)/module/sipp/sipp_setting.mk
    include $(XEYE_PLATFORM)/module/sipp/sipp_build.mk
    $(info "xeye platform include sipp RGB module on leon LRT")
else 
    $(info "xeye platform don't support sipp RGB module")
endif

ifeq ($(CONFIG_SIPP_YUV),lrt)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/sipp/yuv
    include $(XEYE_PLATFORM)/module/sipp/sipp_setting.mk
    include $(XEYE_PLATFORM)/module/sipp/sipp_build.mk
    $(info "xeye platform include sipp YUV module on leon LRT")
else
    $(info "xeye platform don't support sipp YUV module")
endif

ifeq ($(CONFIG_CJSON),all)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/cJSON
    ComponentList_LOS += $(XEYE_PLATFORM)/module/cJSON
    $(info "xeye platform include cJSON module on LOS and LRT")
else ifeq ($(CONFIG_CJSON),los)
    ComponentList_LOS += $(XEYE_PLATFORM)/module/cJSON
    $(info "xeye platform include cJSON module on leon LOS")
else ifeq ($(CONFIG_CJSON),lrt)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/cJSON
    $(info "xeye platform include cJSON module on leon LRT")
else
    $(info "xeye platform don't config cJSON module")
endif

ifeq ($(CONFIG_VSC),los)
    # We cannot add usb component to LRT, or compile fails
    ComponentList_LOS += $(XEYE_PLATFORM)/module/vsc
    LEON_APP_URC_SOURCES += $(wildcard $(XEYE_PLATFORM)/module/vsc/leon/*.urc)
    $(info "xeye platform include vsc module")
else
    $(info "xeye platform don't support vsc module")
endif

ifeq ($(CONFIG_VSC_XLINK),los)
    # We cannot add usb component to LRT, or compile fails
    ComponentList_LOS += $(XEYE_PLATFORM)/module/vsc_xlink
    LEON_APP_URC_SOURCES += $(wildcard $(XEYE_PLATFORM)/module/vsc_xlink/leon/vsc/*.urc)
    $(info "xeye platform include vsc_xlink module")

else
    $(info "xeye platform don't support vsc_xlink module")
endif

ifeq ($(CONFIG_PLOG),los)
    ComponentList_LOS += $(XEYE_PLATFORM)/module/plog
    $(info "xeye platform inlude plog module")
else
    $(info "xeye platform don't support plog module")
endif

ifeq ($(CONFIG_ZBASE64),all)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/zbase64
    ComponentList_LOS += $(XEYE_PLATFORM)/module/zbase64
    $(info "xeye platform include zbase64 module on LOS and LRT")
else ifeq ($(CONFIG_ZBASE64),los)
    ComponentList_LOS += $(XEYE_PLATFORM)/module/zbase64
    $(info "xeye platform include zbase64 module on leon LOS")
else ifeq ($(CONFIG_ZBASE64),lrt)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/zbase64
    $(info "xeye platform include zbase64 module on leon LRT")
else
    $(info "xeye platform don't config zbase64 module")
endif

ifeq ($(CONFIG_MICROTAR),all)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/microtar
    ComponentList_LOS += $(XEYE_PLATFORM)/module/microtar
    $(info "xeye platform include microtar module on LOS and LRT")
else ifeq ($(CONFIG_MICROTAR),los)
    ComponentList_LOS += $(XEYE_PLATFORM)/module/microtar
    $(info "xeye platform include microtar module on leon LOS")
else ifeq ($(CONFIG_MICROTAR),lrt)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/microtar
    $(info "xeye platform include microtar module on leon LRT")
else
    $(info "xeye platform don't config microtar module")
endif


ifeq ($(CONFIG_LIBJPEG),all)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/libjpeg
    ComponentList_LOS += $(XEYE_PLATFORM)/module/libjpeg
    $(info "xeye platform include libjpeg module on LOS and LRT")
else ifeq ($(CONFIG_LIBJPEG),los)
    ComponentList_LOS += $(XEYE_PLATFORM)/module/libjpeg
    $(info "xeye platform include libjpeg module on leon LOS")
else ifeq ($(CONFIG_LIBJPEG),lrt)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/libjpeg
    $(info "xeye platform include libjpeg module on leon LRT")
else
    $(info "xeye platform don't config libjpeg module")
endif

ifeq ($(CONFIG_EIGEN),all)
    ComponentList_LOS += $(XEYE_PLATFORM)/module/Eigen
    ComponentList_LRT += $(XEYE_PLATFORM)/module/Eigen
    $(info "xeye platform include Eigen module on LOS and LRT")
else ifeq ($(CONFIG_EIGEN),los)
    ComponentList_LOS += $(XEYE_PLATFORM)/module/Eigen
    $(info "xeye platform include Eigen module on leon LOS")
else ifeq ($(CONFIG_EIGEN),lrt)
    ComponentList_LRT += $(XEYE_PLATFORM)/module/Eigen
    $(info "xeye platform include Eigen module on leon LRT")
else
    $(info "xeye platform don't config Eigen module")
endif


ifeq ($(CONFIG_ETH),los)
    ComponentList_LOS += $(XEYE_PLATFORM)/module/eth
    $(info "xeye platform include eth module on LOS")
else
    $(info "xeye platform don't eth Eigen module")
endif

CCOPT += -DRES_WIDTH=$(RES_WIDTH) -DRES_HEIGHT=$(RES_HEIGHT)
CCOPT_LRT += -DRES_WIDTH=$(RES_WIDTH) -DRES_HEIGHT=$(RES_HEIGHT)
# The maximum of graph number, defined in the upper app's makefile
CCOPT += -DMAX_GRAPH=$(MAX_GRAPH)
CCOPT_LRT += -DMAX_GRAPH=$(MAX_GRAPH)
