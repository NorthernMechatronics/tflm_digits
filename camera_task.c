/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2023, Northern Mechatronics, Inc.
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

#define COMMAND_BUFFER_LEN (64)

static ArducamCamera camera;
static uint8_t command_buffer[COMMAND_BUFFER_LEN];
static uint8_t command_length;
static uint8_t sendFlag = TRUE;
static uint32_t camera_stream_read = 0;
static uint8_t camera_stream_started = 0;

static TaskHandle_t camera_task_handle;

static void camera_process_host_command(uint8_t ch)
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
        camera_stream_started = 1;
        camera_stream_read = 0;
        arducamUartWrite(0xff);
        arducamUartWrite(0xAA);
        arducamUartWrite(0x01);
        arducamUartWrite((uint8_t)(camera.totalLength & 0xff));
        arducamUartWrite((uint8_t)((camera.totalLength >> 8) & 0xff));
        arducamUartWrite((uint8_t)((camera.totalLength >> 16) & 0xff));
        arducamUartWrite((uint8_t)((camera.receivedLength >> 24) & 0xff));
        arducamUartWrite(camera.currentPixelFormat);
    }
    if (camera_stream_started == 1)
    {
        camera_stream_read += length;
        arducamUartWriteBuff(image, length);
    }
    if (camera_stream_read == camera.totalLength)
    {
        camera_stream_started = 0;
        arducamUartWrite(0xff);
        arducamUartWrite(0xBB);
    }
    return sendFlag;
}

static void camera_stop_preview(void)
{
    camera_stream_read = 0;
    camera_stream_started    = 0;
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
    console_register_custom_process(camera_process_host_command);
    command_length = 0;
    memset(command_buffer, 0, COMMAND_BUFFER_LEN);
    camera = createArducamCamera(1);
    begin(&camera);
    registerCallback(&camera, camera_read_buffer, 200, camera_stop_preview);
}

static void camera_task(void *parameter)
{
    camera_setup();
    while (1)
    {
        captureThread(&camera);
        vTaskDelay(20);
    }
}

void camera_task_create(uint32_t priority)
{
    xTaskCreate(camera_task, "camera", 512, 0, priority, &camera_task_handle);
}