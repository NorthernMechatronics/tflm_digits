VPATH += $(RTOS)/kernel
VPATH += $(RTOS)/cli
VPATH += ./rtos/FreeRTOS/portable

RTOS_SRC += croutine.c
RTOS_SRC += event_groups.c
RTOS_SRC += list.c
RTOS_SRC += queue.c
RTOS_SRC += stream_buffer.c
RTOS_SRC += tasks.c
RTOS_SRC += timers.c
RTOS_SRC += heap_4.c
RTOS_SRC += port.c

RTOS_SRC += FreeRTOS_CLI.c