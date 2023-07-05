include makedefs/defs_lorawan.mk
include makedefs/includes_lorawan.mk
include makedefs/sources_lorawan.mk

LORAWAN_OBJS_DBG += $(LORAWAN_SRC:%.c=$(BUILDDIR_DBG)/%.o)
LORAWAN_DEPS_DBG += $(LORAWAN_SRC:%.c=$(BUILDDIR_DBG)/%.d)

LORAWAN_OBJS_REL += $(LORAWAN_SRC:%.c=$(BUILDDIR_REL)/%.o)
LORAWAN_DEPS_REL += $(LORAWAN_SRC:%.c=$(BUILDDIR_REL)/%.d)

lorawan_install: lorawan_install_dbg lorawan_install_rel

lorawan_install_dbg: $(INSTALLDIR)/$(LORAWAN_LIB_DBG)

lorawan_install_rel: $(INSTALLDIR)/$(LORAWAN_LIB_REL)

$(INSTALLDIR)/$(LORAWAN_LIB_DBG): $(BUILDDIR_DBG)/$(LORAWAN_LIB_DBG)
	$(CP) $< $@

$(INSTALLDIR)/$(LORAWAN_LIB_REL): $(BUILDDIR_REL)/$(LORAWAN_LIB_REL)
	$(CP) $< $@

lorawan_dbg: $(BUILDDIR_DBG)/$(LORAWAN_LIB_DBG)

$(BUILDDIR_DBG)/$(LORAWAN_LIB_DBG): $(LORAWAN_OBJS_DBG)
	$(AR) rsvc $@ $^

$(LORAWAN_OBJS_DBG): $(BUILDDIR_DBG)/%.o : %.c $(LORAWAN_CONFIG)
	$(CC) -c $(CFLAGS_DBG) $(LORAWAN_DEFINES) $(LORAWAN_INC) $(HAL_INC) $< -o $@

lorawan_rel: $(BUILDDIR_REL)/$(LORAWAN_LIB_REL)

$(BUILDDIR_REL)/$(LORAWAN_LIB_REL): $(LORAWAN_OBJS_REL)
	$(AR) rsvc $@ $^

$(LORAWAN_OBJS_REL): $(BUILDDIR_REL)/%.o : %.c $(LORAWAN_CONFIG)
	$(CC) -c $(CFLAGS_REL) $(LORAWAN_DEFINES) $(LORAWAN_INC) $(HAL_INC) $< -o $@

-include $(LORAWAN_DEPS_DBG)
-include $(LORAWAN_DEPS_REL)
