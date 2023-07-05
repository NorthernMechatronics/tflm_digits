BLE := $(SDK_ROOT)/comms/ble
BLE_LIB_DBG  := libble$(SUFFIX_DBG).a
BLE_LIB_REL  := libble$(SUFFIX_REL).a
BLE_CONFIG ?= $(BLE)/../../targets/nm180100/comms/ble/wsf/include/ble_config.h