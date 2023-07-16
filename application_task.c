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
#include <string.h>

#include <am_mcu_apollo.h>
#include <am_util.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <timers.h>

#include <am_bsp.h>

#include "tflm.h"

#include "button.h"
#include "camera_task.h"

#include "application_task.h"
#include "application_task_cli.h"

static TaskHandle_t application_task_handle;
static TimerHandle_t application_timer_handle;
static QueueHandle_t application_queue_handle;

static am_hal_burst_avail_e application_burst_available;
static am_hal_burst_mode_e  application_burst_mode;

static uint8_t *image_buffer;
static size_t image_size;

typedef enum application_command_e
{
    APPLICATION_COMMAND_CAPTURE_START,
    APPLICATION_COMMAND_CAPTURE_DONE,
    APPLICATION_COMMAND_HEARTBEAT
} application_command_t;

void application_task_send(application_command_t *message);

static void application_timer_handler(TimerHandle_t timer)
{
    application_command_t command;
    command = APPLICATION_COMMAND_HEARTBEAT;
    application_task_send(&command);
}

static void application_button_handler()
{
    application_command_t command;
    command = APPLICATION_COMMAND_CAPTURE_START;
    application_task_send(&command);
}

static void application_camera_handler(uint8_t *buffer, size_t size)
{
    image_buffer = buffer;
    image_size = size;

    application_command_t command;
    command = APPLICATION_COMMAND_CAPTURE_DONE;
    application_task_send(&command);
}

static void application_burst_init()
{
    am_hal_burst_mode_initialize(&application_burst_available);
    if (application_burst_available == AM_HAL_BURST_AVAIL)
    {
        am_hal_burst_mode_disable(&application_burst_mode);
    }
}

static void application_burst_enable()
{
    if (application_burst_available == AM_HAL_BURST_AVAIL)
    {
        am_hal_burst_mode_enable(&application_burst_mode);
    }
}

static void application_burst_disable()
{
    if (application_burst_available == AM_HAL_BURST_AVAIL)
    {
        am_hal_burst_mode_disable(&application_burst_mode);
    }
}

static void application_inference()
{
    uint8_t *result;
    size_t result_size;
    application_burst_enable();
    tflm_inference(image_buffer, image_size, result, &result_size);
    application_burst_disable();
    am_util_stdio_printf("Inference Done\r\n");
}

static void application_setup_task()
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED0, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(AM_BSP_GPIO_LED0, AM_HAL_GPIO_OUTPUT_CLEAR);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED1, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(AM_BSP_GPIO_LED1, AM_HAL_GPIO_OUTPUT_CLEAR);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED2, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(AM_BSP_GPIO_LED2, AM_HAL_GPIO_OUTPUT_CLEAR);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED3, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(AM_BSP_GPIO_LED3, AM_HAL_GPIO_OUTPUT_CLEAR);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED4, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(AM_BSP_GPIO_LED4, AM_HAL_GPIO_OUTPUT_CLEAR);

    application_burst_init();
    tflm_setup();

    button_sequence_register(2, 0B00, application_button_handler);
    camera_event_subscribe(CAMERA_COMMAND_STILL_RETRIEVE_DONE, application_camera_handler);

    xTimerStart(application_timer_handle, portMAX_DELAY);
}

static void application_task(void *parameter)
{
    application_command_t message;

    application_task_cli_register();
    application_setup_task();
    while (1)
    {
        if (xQueueReceive(application_queue_handle, &message, portMAX_DELAY) == pdPASS)
        {
            switch(message)
            {
            case APPLICATION_COMMAND_CAPTURE_START:
                am_util_stdio_printf("Capture Start Triggered\r\n");
                break;

            case APPLICATION_COMMAND_CAPTURE_DONE:
                am_util_stdio_printf("Capture Done\r\n");
                application_inference();
                break;

            case APPLICATION_COMMAND_HEARTBEAT:
                am_hal_gpio_state_write(AM_BSP_GPIO_LED0, AM_HAL_GPIO_OUTPUT_TOGGLE);
                break;

            default:
                break;
            }
        }
    }
}

void application_task_create(uint32_t priority)
{
    application_queue_handle = xQueueCreate(8, sizeof(application_command_t));
    application_timer_handle = xTimerCreate("application", pdMS_TO_TICKS(500), pdTRUE, NULL, application_timer_handler);
    xTaskCreate(application_task, "application", 512, 0, priority, &application_task_handle);
}

void application_task_send(application_command_t *message)
{
    if (application_queue_handle)
    {
        BaseType_t xHigherPriorityTaskWoken;

        if (xPortIsInsideInterrupt() == pdTRUE)
        {
            xHigherPriorityTaskWoken = pdFALSE;
            xQueueSendFromISR(application_queue_handle, message, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else
        {
            xQueueSend(application_queue_handle, message, portMAX_DELAY);
        }
    }
}