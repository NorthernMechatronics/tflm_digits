/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2022, Northern Mechatronics, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <am_mcu_apollo.h>
#include <am_util.h>

#include <FreeRTOS.h>
#include <list.h>
#include <task.h>
#include <queue.h>
#include <timers.h>

#include "am_bsp.h"
#include "button.h"
#include "button_task.h"

#define BUTTON_DEBOUNCE_DELAY_MS   (20)
#define BUTTON_PRESS_SAMPLING_MS   (20)
#define BUTTON_PRESS_GAP_MS       (500)
#define BUTTON_PRESS_SHORT_MS     (500)
#define BUTTON_PRESS_LONG_MS     (2000)

typedef enum
{
    BUTTON_PRESS_SHORT = 0,
    BUTTON_PRESS_LONG = 1,
} button_press_e;

typedef enum
{
    BUTTON_STATE_TIMEOUT,
    BUTTON_STATE_PRESSED,
} button_command_e;

typedef struct
{
    uint8_t size:8;
    uint32_t sequence:24;
} button_sequence_t;

typedef union
{
    button_sequence_t sequence;
    uint32_t          value;
} button_sequence_u;


static TaskHandle_t  button_task_handle;
static QueueHandle_t button_queue_handle;
static TimerHandle_t button_timer_handle;
static uint32_t button_state_counter;

static List_t button_sequence_list;
static uint32_t ui32ButtonSequence;

static void button_task_send_command(button_command_e command)
{
    BaseType_t xHigherPriorityTaskWoken;

    if (xPortIsInsideInterrupt() == pdTRUE)
    {
        xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(button_queue_handle, &command, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        xQueueSend(button_queue_handle, &command, portMAX_DELAY);
    }
}

static void button_task_timer_callback(TimerHandle_t xTimer)
{
    button_task_send_command(BUTTON_STATE_TIMEOUT);
}

static void button_press_handler()
{
    am_hal_gpio_interrupt_disable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));

    button_task_send_command(BUTTON_STATE_PRESSED);
}

static button_press_e button_press_classification()
{
    uint32_t initial_state;
    uint32_t current_state;
    uint32_t duration;

    xTimerStop(button_timer_handle, portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_DELAY_MS));
    am_hal_gpio_state_read(AM_BSP_GPIO_BUTTON0, AM_HAL_GPIO_INPUT_READ, &initial_state);
    if (initial_state == 1)
    {
        am_hal_gpio_interrupt_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));
        am_hal_gpio_interrupt_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));
        return BUTTON_PRESS_SHORT;
    }

    duration = BUTTON_DEBOUNCE_DELAY_MS;
    current_state = initial_state;
    while (current_state == initial_state)
    {
        vTaskDelay(pdMS_TO_TICKS(BUTTON_PRESS_SAMPLING_MS));
        am_hal_gpio_state_read(AM_BSP_GPIO_BUTTON0, AM_HAL_GPIO_INPUT_READ, &current_state);
        duration += BUTTON_PRESS_SAMPLING_MS;
    }

    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));

    xTimerChangePeriod(button_timer_handle, pdMS_TO_TICKS(BUTTON_PRESS_GAP_MS), portMAX_DELAY);

    if (duration < BUTTON_PRESS_SHORT_MS)
    {
        return BUTTON_PRESS_SHORT;
    }

    return BUTTON_PRESS_LONG;
}

static void button_sequence_set_bit(uint32_t *pSequence, uint32_t count, button_press_e press)
{
    *pSequence |= (press << count);
}

static void button_sequence_execute()
{
    ListItem_t *pItem = listGET_HEAD_ENTRY(&button_sequence_list);

    while (pItem != (ListItem_t *)&(button_sequence_list.xListEnd))
    {
        button_sequence_u seq = (button_sequence_u)pItem->xItemValue;
        if (button_state_counter == seq.sequence.size)
        {
            if (ui32ButtonSequence == seq.sequence.sequence)
            {
                sequence_callback_t callback = (sequence_callback_t)pItem->pvOwner;
                if (callback != NULL)
                {
                    callback();
                    pItem = listGET_NEXT(pItem);
                    continue;
                }
            }
        }
        pItem = listGET_NEXT(pItem);
    }
}

static void button_task(void *parameter)
{
    button_command_e command;
    button_press_e press_type;

    while (1)
    {
        if (xQueueReceive(button_queue_handle, &command, portMAX_DELAY) == pdPASS)
        {
            switch(command)
            {
            case BUTTON_STATE_PRESSED:
                //led_send_command(LED_CMD_BUTTON_PRESS);
                press_type = button_press_classification();
                button_sequence_set_bit(&ui32ButtonSequence, button_state_counter, press_type);
                button_state_counter++;
                break;

            case BUTTON_STATE_TIMEOUT:
                button_sequence_execute();
                button_state_counter = 0;
                ui32ButtonSequence = 0;
                break;

            default:
                button_state_counter = 0;
                ui32ButtonSequence = 0;
                break;
            }
        }
     }
}

void button_task_create(uint32_t priority)
{
    button_state_counter = 0;
    ui32ButtonSequence = 0;

    am_hal_gpio_pinconfig(AM_BSP_GPIO_BUTTON0, g_AM_BSP_GPIO_BUTTON0);
    am_hal_gpio_interrupt_register(AM_BSP_GPIO_BUTTON0, button_press_handler);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));
 

    xTaskCreate(button_task, "Button Task", 512, 0, priority, &button_task_handle);
    button_queue_handle = xQueueCreate(16, sizeof(button_command_e));
    button_timer_handle = xTimerCreate(
        "Button Timer",
        pdMS_TO_TICKS(1000),
        pdFALSE,
        NULL,
        button_task_timer_callback);

    vListInitialise(&button_sequence_list);    
}

void button_sequence_register(uint8_t size, uint32_t value, sequence_callback_t cb)
{
    ListItem_t *pItem = pvPortMalloc(sizeof(ListItem_t));
    vListInitialiseItem(pItem);
    vListInsert(&button_sequence_list, pItem);

    button_sequence_u seq;
    seq.sequence.size     = size;
    seq.sequence.sequence = value;

    pItem->xItemValue = seq.value;
    pItem->pvOwner = cb;
}

void button_sequence_unregister(uint8_t size, uint32_t value, sequence_callback_t cb)
{
    ListItem_t *pItem = listGET_HEAD_ENTRY(&button_sequence_list);

    while (pItem != (ListItem_t *)&(button_sequence_list.xListEnd))
    {
        button_sequence_u seq = (button_sequence_u)pItem->xItemValue;
        if (size == seq.sequence.size)
        {
            if (value == seq.sequence.sequence)
            {
                sequence_callback_t callback = (sequence_callback_t)pItem->pvOwner;
                if (callback == cb)
                {
                    uxListRemove(pItem);
                    vPortFree(pItem);
                    return;
                }
            }
        }
        pItem = listGET_NEXT(pItem);
    }
}