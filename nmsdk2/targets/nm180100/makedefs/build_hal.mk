include makedefs/defs_hal.mk
include makedefs/includes_hal.mk
include makedefs/sources_hal.mk

HAL_OBJS_DBG += $(HAL_SRC:%.c=$(BUILDDIR_DBG)/%.o)
HAL_DEPS_DBG += $(HAL_SRC:%.c=$(BUILDDIR_DBG)/%.d)

HAL_OBJS_REL += $(HAL_SRC:%.c=$(BUILDDIR_REL)/%.o)
HAL_DEPS_REL += $(HAL_SRC:%.c=$(BUILDDIR_REL)/%.d)

hal_install: hal_install_dbg hal_install_rel

hal_install_dbg: $(INSTALLDIR)/$(HAL_LIB_DBG)

hal_install_rel: $(INSTALLDIR)/$(HAL_LIB_REL)

$(INSTALLDIR)/$(HAL_LIB_DBG): $(BUILDDIR_DBG)/$(HAL_LIB_DBG)
	$(CP) $< $@

$(INSTALLDIR)/$(HAL_LIB_REL): $(BUILDDIR_REL)/$(HAL_LIB_REL)
	$(CP) $< $@

hal_dbg: $(BUILDDIR_DBG)/$(HAL_LIB_DBG)

$(BUILDDIR_DBG)/$(HAL_LIB_DBG): $(HAL_OBJS_DBG)
	$(AR) rsvc $@ $^

$(HAL_OBJS_DBG): $(BUILDDIR_DBG)/%.o : %.c
	$(CC) -c $(CFLAGS_DBG) $(HAL_INC) $< -o $@

hal_rel: $(BUILDDIR_REL)/$(HAL_LIB_REL)

$(BUILDDIR_REL)/$(HAL_LIB_REL): $(HAL_OBJS_REL)
	$(AR) rsvc $@ $^

$(HAL_OBJS_REL): $(BUILDDIR_REL)/%.o : %.c
	$(CC) -c $(CFLAGS_REL) $(HAL_INC) $< -o $@

-include $(HAL_DEPS_DBG)
-include $(HAL_DEPS_REL)
