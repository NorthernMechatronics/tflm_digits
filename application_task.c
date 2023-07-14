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
#include <task.h>

#include <am_bsp.h>

#include "ArducamCamera.h"
#include "ArducamLink.h"

#include "console_task.h"

#include "application_task.h"
#include "application_task_cli.h"

#include "tflm.h"

#define COMMAND_BUFFER_LEN (64)

static ArducamCamera camera;
static uint8_t command_buffer[COMMAND_BUFFER_LEN];
static uint8_t command_length;
static uint8_t sendFlag = TRUE;
static uint32_t readImageLength = 0;
static uint8_t jpegHeadFlag = 0;

static TaskHandle_t application_task_handle;

static void camera_process(uint8_t ch)
{
    command_buffer[command_length] = ch;
    command_length++;
    if (ch == 0xAA)
    {
        uartCommandProcessing(&camera, &command_buffer[1]);
        command_length = 0;
        memset(command_buffer, 0, COMMAND_BUFFER_LEN);
    }
}

static uint8_t camera_read_buffer(uint8_t *image, uint8_t length)
{
    if (image[0] == 0xff && image[1] == 0xd8)
    {
        jpegHeadFlag = 1;
        readImageLength = 0;
        arducamUartWrite(0xff);
        arducamUartWrite(0xAA);
        arducamUartWrite(0x01);
        arducamUartWrite((uint8_t)(camera.totalLength & 0xff));
        arducamUartWrite((uint8_t)((camera.totalLength >> 8) & 0xff));
        arducamUartWrite((uint8_t)((camera.totalLength >> 16) & 0xff));
        arducamUartWrite((uint8_t)((camera.receivedLength >> 24) & 0xff));
        arducamUartWrite(camera.currentPixelFormat);
    }
    if (jpegHeadFlag == 1)
    {
        readImageLength += length;
        arducamUartWriteBuff(image, length);
    }
    if (readImageLength == camera.totalLength)
    {
        jpegHeadFlag = 0;
        arducamUartWrite(0xff);
        arducamUartWrite(0xBB);
    }
    return sendFlag;
}

static void camera_stop_preview(void)
{
    readImageLength = 0;
    jpegHeadFlag    = 0;
    uint32_t len    = 9;

    arducamUartWrite(0xff);
    arducamUartWrite(0xBB);
    arducamUartWrite(0xff);
    arducamUartWrite(0xAA);
    arducamUartWrite(0x06);
    arducamUartWriteBuff((uint8_t*)&len, 4);
    am_util_stdio_printf("streamoff");
    arducamUartWrite(0xff);
    arducamUartWrite(0xBB);
}

static void camera_setup()
{
    console_register_custom_process_trigger(0x55, 0xAA);
    console_register_custom_process(camera_process);
    command_length = 0;
    memset(command_buffer, 0, COMMAND_BUFFER_LEN);
    camera = createArducamCamera(1);
    begin(&camera);
    registerCallback(&camera, camera_read_buffer, 200, camera_stop_preview);
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
}

static void application_task(void *parameter)
{
    application_task_cli_register();

    application_setup_task();
    camera_setup();
    while (1)
    {
        captureThread(&camera);
        vTaskDelay(50);
        am_hal_gpio_state_write(AM_BSP_GPIO_LED0, AM_HAL_GPIO_OUTPUT_TOGGLE);
    }
}

void application_task_create(uint32_t priority)
{
    xTaskCreate(application_task, "application", 512, 0, priority, &application_task_handle);
}