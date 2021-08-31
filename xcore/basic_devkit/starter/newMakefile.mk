MDK_ROOT =  ../../../xos/mdk
MDK_INSTALL_DIR  ?= $(MDK_ROOT)/common

include $(MDK_INSTALL_DIR)/../system/build/mdk.mk

TEST_TYPE   := "MANUAL"
TEST_TAGS   := "MA2480"
TEST_HW_PLATFORM = "MV0235_MA2480"

