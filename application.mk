#******************************************************************************
#
# Step 1
# Define the locations of the various SDKs and libraries.
#
#******************************************************************************
NMSDK    ?= $(shell pwd)/nmsdk2
TARGET   := $(NMSDK)/targets/nm180100
LDSCRIPT := AMA3B1KK.ld

#******************************************************************************
#
# Step 2
# Specify the location of the board support package to be used.
#
#******************************************************************************
BSP_DIR := ./bsp/nm180100evb
BSP_SRC := bsp_pins.src

#******************************************************************************
#
# Step 3
# Specify output version and name
#
#******************************************************************************
OUTPUT_VERSION := 0x00

OUTPUT      := nmapp
OUTPUT_OTA  := nmapp-ota
OUTPUT_WIRE := nmapp-wire
UPDATE_STORAGE_ADDRESS := 0x80000

#******************************************************************************
#
# Step 4
# Specify SDK custom configurations here.  Use $(shell pwd) expansion as an
# absolute path is required.  The SDK must be rebuilt manually the first time a
# configuration file is overridden.
#
#   make clean-sdk
#
# For example: 
#   FREERTOS_CONFIG := $(shell pwd)/config/FreeRTOSConfig.h
#
# Current overridable configurations are:
#   FREERTOS_CONFIG
#	LORAWAN_CONFIG
#	BLE_CONFIG
#
#******************************************************************************
# FREERTOS_CONFIG := $(shell pwd)/config/FreeRTOSConfig.h
# LORAWAN_CONFIG  := $(shell pwd)/config/lorawan_config.h
# BLE_CONFIG      := $(shell pwd)/config/ble_config.h

#******************************************************************************
#
# Step 5
# Include additional source, header, libraries or paths below.
#
# Examples:
#	DEFINES  += -Dadditional_defines
#   INCLUDES += -Iadditional_include_path
#   VPATH    += additional_source_path
#******************************************************************************
DEFINES += -DRAT_LORAWAN_ENABLE
DEFINES += -DRAT_BLE_ENABLE

INCLUDES += -I.
INCLUDES += -I./config

VPATH += .

SRC += startup_gcc.c
SRC += main.c
SRC += console_task.c
SRC += application_task.c
SRC += application_task_cli.c


DEFINES += -DSOFT_SE
DEFINES += -DCONTEXT_MANAGEMENT_ENABLED

INCLUDES += -I./comms/lorawan/common/LmHandler/packages
INCLUDES += -I./comms/lorawan/common/LmHandler
INCLUDES += -I./comms/lorawan/common
INCLUDES += -I./comms/lorawan/soft-se
INCLUDES += -I./comms/lorawan

VPATH += ./comms/lorawan/common/LmHandler/packages
VPATH += ./comms/lorawan/common/LmHandler
VPATH += ./comms/lorawan/common
VPATH += ./comms/lorawan/soft-se
VPATH += ./comms/lorawan

SRC += aes.c
SRC += cmac.c
SRC += soft-se.c
SRC += lmh_callbacks.c
SRC += lmhp_fragmentation.c
SRC += lorawan_se.c
SRC += lorawan_task.c
SRC += lorawan_task_cli.c


INCLUDES += -I./comms/ble
INCLUDES += -I./comms/ble/amota
INCLUDES += -I./comms/ble/amota/profile
INCLUDES += -I./comms/ble/amota/service
INCLUDES += -I./utils/bootloader

VPATH += ./comms/ble
VPATH += ./comms/ble/amota
VPATH += ./comms/ble/amota/profile
VPATH += ./comms/ble/amota/service
VPATH += ./utils/bootloader

SRC += ble_task.c
SRC += ble_task_cli.c
SRC += ble_stack.c
SRC += amota_main.c
SRC += amotas_main.c
SRC += svc_amotas.c

SRC += am_bootloader.c
SRC += am_multi_boot.c

INCLUDES += -I./utils/RTT/RTT
INCLUDES += -I./utils/RTT/Config

VPATH += ./utils/RTT/RTT

SRC += SEGGER_RTT.c
SRC += SEGGER_RTT_printf.c