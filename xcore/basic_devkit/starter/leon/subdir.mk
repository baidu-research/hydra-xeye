ccopt-los-y += -D'FLIC_2LEONS'
ccopt-los-y += -D'DYN_IRQ_ASSIGNMENT=1'
ccopt-los-y += -D'PLL_DESIRED_FREQ_KHZ=600000'
ccopt-los-y += -D'DEFAULT_OSC0_KHZ=24000'
ccopt-los-y += -D'FRAME_TIME=33'
ccopt-los-y += -D'PLG_ISP_MAX_W=4096'
ccopt-los-y += -D'PLG_ISP_MAX_H=3040'
ccopt-los-y += -D'MIPIRX_DISABLE_WAIT_LPSTATUS_IN=1'
ccopt-los-y+=-D'N_POOL_FRMS_RAW=5'
ccopt-los-y+=-D'N_POOL_FRMS_VDO=3'
ccopt-los-y+=-D'N_POOL_FRMS_STL=2'
ccopt-los-y+=-D'DEF_POOL_SZ=1024'
ccopt-los-y+=-D'MAX_ZSL_BUF_SIZE=3'
ccopt-los-y+=-D'FBDEVICE_WMC'
ccopt-los-y+=-D'DEFAULT_ALLOC_TIMEOUT_MS=4000'
ccopt-los-y+=-D__MMS_DEBUG__
ccopt-los-y+=-D___RTEMS___
ccopt-los-y+=-DSPI_SLAVE
ccopt-los-y+=-DCAMERA_INTERFACE_ANDROID_CAMERA3
ccopt-los-y+=-D'GZZ_AEWB_MERGER_SWITCH=0'
ifeq "y" "$(CONFIG_BUILD_FOR_TUNINGTOOL)"
ccopt-los-y+=-D'DTP_DB_NAME="database_IMX378_OV9282mono_4tt.bin"'
else
ccopt-los-y+=-D'DTP_DB_NAME="database_IMX378_OV9282mono.bin"'
endif
ccopt-los-$(CONFIG_BUILD_FOR_POWER_MEASUREMENT)+= -D'POWER_MEASUREMENT'
ccopt-los-y+=-DDEBUG

srcs-los-y += ic_main.c \
  app_guzzi_command_dbg.c \
  main.cpp \
  gzz_main.c \
  rtems_config.c \
  serialPort.c \
  boardconfig.c \
  app_guzzi_command_usb.c \
  serialPort_create.c \
  camera_control.c \
  app_guzzi_command_spi.c \
  dtp_database.S
