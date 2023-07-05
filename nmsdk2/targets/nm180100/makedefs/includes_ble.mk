BLE_DEFINES += -DWDXS_INCLUDED=1
BLE_DEFINES += -DSEC_CMAC_CFG=1
BLE_DEFINES += -DSEC_ECC_CFG=2
BLE_DEFINES += -DSEC_CCM_CFG=1
BLE_DEFINES += -DHCI_TR_UART=1
#BLE_DEFINES += -DWSF_CS_STATS=1
#BLE_DEFINES += -DWSF_BUF_STATS=1
BLE_DEFINES += -DWSF_TRACE_ENABLED=1
#BLE_DEFINES += -DWSF_ASSERT_ENABLED=1

BLE_INC += -I$(BLE)/ble-profiles/include
BLE_INC += -I$(BLE)/ble-profiles/include/app
BLE_INC += -I$(BLE)/ble-profiles/sources/apps/app

BLE_INC += -I$(BLE)/ble-profiles/sources/profiles/include
BLE_INC += -I$(BLE)/ble-profiles/sources/profiles

BLE_INC += -I$(BLE)/ble-profiles/sources/services

BLE_INC += -I$(BLE)/ble-host/include
BLE_INC += -I$(BLE)/ble-host/sources/stack/att
BLE_INC += -I$(BLE)/ble-host/sources/stack/cfg
BLE_INC += -I$(BLE)/ble-host/sources/stack/dm
BLE_INC += -I$(BLE)/ble-host/sources/stack/hci
BLE_INC += -I$(BLE)/ble-host/sources/stack/l2c
BLE_INC += -I$(BLE)/ble-host/sources/stack/smp
#BLE_INC += -I$(BLE)/ble-host/sources/hci/dual_chip

BLE_INC += -I$(BLE)/../../targets/nm180100/comms/ble/wsf/include
BLE_INC += -I$(BLE)/../../targets/nm180100/comms/ble/wsf/include/util
BLE_INC += -I$(BLE)/../../targets/nm180100/comms/ble/ble-host/sources/hci/nm180100
BLE_INC += -I$(BLE)/../../targets/nm180100/comms/ble/ble-host/sources/hci/nm180100/apollo3
BLE_INC += -I$(BLE)/thirdparty/uecc