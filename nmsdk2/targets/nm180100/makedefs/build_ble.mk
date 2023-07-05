include makedefs/defs_ble.mk
include makedefs/includes_ble.mk
include makedefs/sources_ble.mk

BLE_OBJS_DBG += $(BLE_SRC:%.c=$(BUILDDIR_DBG)/%.o)
BLE_DEPS_DBG += $(BLE_SRC:%.c=$(BUILDDIR_DBG)/%.d)

BLE_OBJS_REL += $(BLE_SRC:%.c=$(BUILDDIR_REL)/%.o)
BLE_DEPS_REL += $(BLE_SRC:%.c=$(BUILDDIR_REL)/%.d)

ble_install: ble_install_dbg ble_install_rel

ble_install_dbg: $(INSTALLDIR)/$(BLE_LIB_DBG)

ble_install_rel: $(INSTALLDIR)/$(BLE_LIB_REL)

$(INSTALLDIR)/$(BLE_LIB_DBG): $(BUILDDIR_DBG)/$(BLE_LIB_DBG)
	$(CP) $< $@

$(INSTALLDIR)/$(BLE_LIB_REL): $(BUILDDIR_REL)/$(BLE_LIB_REL)
	$(CP) $< $@

ble_dbg: $(BUILDDIR_DBG)/$(BLE_LIB_DBG)

$(BUILDDIR_DBG)/$(BLE_LIB_DBG): $(BLE_OBJS_DBG)
	$(AR) rsvc $@ $^

$(BLE_OBJS_DBG): $(BUILDDIR_DBG)/%.o : %.c $(BLE_CONFIG)
	$(CC) -c $(CFLAGS_DBG) $(BLE_DEFINES) $(BLE_INC) $(HAL_INC) $< -o $@

ble_rel: $(BUILDDIR_REL)/$(BLE_LIB_REL)

$(BUILDDIR_REL)/$(BLE_LIB_REL): $(BLE_OBJS_REL)
	$(AR) rsvc $@ $^

$(BLE_OBJS_REL): $(BUILDDIR_REL)/%.o : %.c $(BLE_CONFIG)
	$(CC) -c $(CFLAGS_REL) $(BLE_DEFINES) $(BLE_INC) $(HAL_INC) $< -o $@

-include $(BLE_DEPS_DBG)
-include $(BLE_DEPS_REL)

