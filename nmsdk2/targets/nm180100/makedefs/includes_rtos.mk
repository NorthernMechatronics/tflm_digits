RTOS_DEFINES += -DAM_FREERTOS

RTOS_INC += -I$(RTOS)/kernel/include
RTOS_INC += -I$(RTOS)/cli
RTOS_INC += -I$(RTOS)/../../targets/nm180100/rtos/FreeRTOS/portable
RTOS_INC += -I$(dir $(FREERTOS_CONFIG))
