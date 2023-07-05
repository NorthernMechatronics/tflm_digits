SDK_ROOT   ?= ../..
INSTALLDIR := ./lib

include makedefs/common.mk

all: debug release

install: debug release $(INSTALLDIR) hal_install rtos_install lorawan_install ble_install

$(INSTALLDIR):
	$(MKDIR) -p "$@"

debug: $(BUILDDIR_DBG) hal_dbg rtos_dbg lorawan_dbg ble_dbg

$(BUILDDIR_DBG):
	$(MKDIR) -p "$@"

release: $(BUILDDIR_REL) hal_rel rtos_rel lorawan_rel ble_rel

$(BUILDDIR_REL):
	$(MKDIR) -p "$@"

include makedefs/build_hal.mk
include makedefs/build_rtos.mk
include makedefs/build_lorawan.mk
include makedefs/build_ble.mk

clean:
	$(RM) -rf ./build

uninstall:
	$(RM) -rf ./lib