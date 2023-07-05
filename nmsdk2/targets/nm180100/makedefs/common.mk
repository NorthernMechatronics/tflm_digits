TOOLCHAIN ?= arm-none-eabi
PART       = apollo3
CPU        = cortex-m4
FPU        = fpv4-sp-d16
FABI       = hard

#### Required Executables ####
SHELL     := /bin/bash
CC   = $(TOOLCHAIN)-gcc
GCC  = $(TOOLCHAIN)-gcc
CPP  = $(TOOLCHAIN)-cpp
LD   = $(TOOLCHAIN)-ld
OCP  = $(TOOLCHAIN)-objcopy
OD   = $(TOOLCHAIN)-objdump
RD   = $(TOOLCHAIN)-readelf
AR   = $(TOOLCHAIN)-ar
SIZE = $(TOOLCHAIN)-size
PYTHON = python

SED   = sed
MKDIR = mkdir
CP    = cp
RM    = rm

DEFINES += -Dgcc
DEFINES += -DAM_PART_APOLLO3
DEFINES += -DAM_PACKAGE_BGA
DEFINES += -DPART_apollo3

DEFINES_DBG += -DAM_ASSERT_INVALID_THRESHOLD=0
DEFINES_DBG += -DAM_DEBUG_ASSERT
DEFINES_DBG += -DAM_DEBUG_PRINTF

CFLAGS  = -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
CFLAGS += -ffunction-sections -fdata-sections -fomit-frame-pointer
CFLAGS += -MMD -MP -std=c99 -Wall
CFLAGS += $(DEFINES)

CFLAGS_DBG += $(CFLAGS)
CFLAGS_DBG += $(DEFINES_DBG)
CFLAGS_DBG += -g -O0

CFLAGS_REL += $(CFLAGS)
CFLAGS_REL += -Os

LFLAGS  = -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
LFLAGS += -static

OCPFLAGS = -Obinary
ODFLAGS  = -S

BUILDDIR_DBG := ./build/debug
SUFFIX_DBG   := -dbg
BUILDDIR_REL := ./build/release
SUFFIX_REL   := -rel

