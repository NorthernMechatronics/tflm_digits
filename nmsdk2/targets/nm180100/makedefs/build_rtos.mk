include makedefs/defs_rtos.mk
include makedefs/includes_hal.mk
include makedefs/includes_rtos.mk
include makedefs/sources_rtos.mk

RTOS_OBJS_DBG += $(RTOS_SRC:%.c=$(BUILDDIR_DBG)/%.o)
RTOS_DEPS_DBG += $(RTOS_SRC:%.c=$(BUILDDIR_DBG)/%.d)

RTOS_OBJS_REL += $(RTOS_SRC:%.c=$(BUILDDIR_REL)/%.o)
RTOS_DEPS_REL += $(RTOS_SRC:%.c=$(BUILDDIR_REL)/%.d)

rtos_install: rtos_install_dbg rtos_install_rel

rtos_install_dbg: $(INSTALLDIR)/$(RTOS_LIB_DBG)

rtos_install_rel: $(INSTALLDIR)/$(RTOS_LIB_REL)

$(INSTALLDIR)/$(RTOS_LIB_DBG): $(BUILDDIR_DBG)/$(RTOS_LIB_DBG)
	$(CP) $< $@

$(INSTALLDIR)/$(RTOS_LIB_REL): $(BUILDDIR_REL)/$(RTOS_LIB_REL)
	$(CP) $< $@

rtos_dbg: $(BUILDDIR_DBG)/$(RTOS_LIB_DBG)

$(BUILDDIR_DBG)/$(RTOS_LIB_DBG): $(RTOS_OBJS_DBG)
	$(AR) rsvc $@ $^

$(RTOS_OBJS_DBG): $(BUILDDIR_DBG)/%.o : %.c $(FREERTOS_CONFIG)
	$(CC) -c $(CFLAGS_DBG) $(RTOS_INC) $(HAL_INC) $< -o $@

rtos_rel: $(BUILDDIR_REL)/$(RTOS_LIB_REL)

$(BUILDDIR_REL)/$(RTOS_LIB_REL): $(RTOS_OBJS_REL)
	$(AR) rsvc $@ $^

$(RTOS_OBJS_REL): $(BUILDDIR_REL)/%.o : %.c $(FREERTOS_CONFIG)
	$(CC) -c $(CFLAGS_REL) $(RTOS_INC) $(HAL_INC) $< -o $@

-include $(RTOS_DEPS_DBG)
-include $(RTOS_DEPS_REL)
