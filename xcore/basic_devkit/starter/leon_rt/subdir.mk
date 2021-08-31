ccopt-lrt-y += -D'FLIC_2LEONS'
ccopt-lrt-y += -D'DYN_IRQ_ASSIGNMENT=1'
ccopt-lrt-y += -D'PLL_DESIRED_FREQ_KHZ=600000'
ccopt-lrt-y += -D'DEFAULT_OSC0_KHZ=24000'
ccopt-lrt-y += -D'FRAME_TIME=33'
ccopt-lrt-y += -D'PLG_ISP_MAX_W=4096'
ccopt-lrt-y += -D'PLG_ISP_MAX_H=3040'
ccopt-lrt-y += -D'MIPIRX_DISABLE_WAIT_LPSTATUS_IN=1'

ccopt-lrt-y+=-D'DEF_POOL_SZ = (6*2*4056*3040)+(3*2*4056*3040)+(2*2*4056*3040)+(3*2*1280*800)+(3*2*1280*800)'
ccopt-lrt-y+=-D'MAX_ZSL_BUF_SIZE=3'

ccopt-lrt-y+=-U CMX_DATA
ccopt-lrt-y+=-DCMX_DATA='__attribute__((section(".cmx.data")))'
ccopt-lrt-y+=-I$(IPIPE_BASE)/components/mod_i2c_command/leon/
ccopt-lrt-y+=-I$(MV_COMMON_BASE)/components/I2CSlave/leon
srcs-lrt-y += rtems_config.c \
  main_rt.cpp
