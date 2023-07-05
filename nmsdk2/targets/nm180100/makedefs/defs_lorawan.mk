LORAWAN	:= $(SDK_ROOT)/comms/lorawan
LORAWAN_LIB_DBG  := liblorawan$(SUFFIX_DBG).a
LORAWAN_LIB_REL  := liblorawan$(SUFFIX_REL).a
LORAWAN_CONFIG ?= $(LORAWAN)/../../targets/nm180100/comms/lorawan/lorawan_config.h