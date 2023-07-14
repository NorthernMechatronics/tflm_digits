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
#include <queue.h>
#include <task.h>
#include <timers.h>

#include <am_bsp.h>

#include "ArducamCamera.h"
#include "ArducamLink.h"

#include "camera_task.h"
#include "console_task.h"

#define COMMAND_BUFFER_LEN (64)

static ArducamCamera camera;
static uint8_t command_buffer[COMMAND_BUFFER_LEN];
static uint8_t command_length;
static uint8_t sendFlag = TRUE;
static uint32_t camera_stream_read = 0;
static uint8_t camera_stream_started = 0;

static TaskHandle_t camera_task_handle;
static QueueHandle_t camera_queue_handle;
static TimerHandle_t camera_timer_handle;

uint8_t camera_process_command(ArducamCamera *cam, uint8_t *command)
{
    camera_message_t message;

    CamStatus state;
    uint16_t gainValue = 0;
    uint32_t exposureValue = 0;
    uint32_t exposureLen1 = 0;
    uint32_t exposureLen2 = 0;
    uint32_t exposureLen3 = 0;

    uint8_t cameraResolution = cam->currentPictureMode;
    uint8_t cameraFarmat = cam->currentPixelFormat;
    switch (command[0])
    {
    case SET_PICTURE_RESOLUTION: // Set Camera Resolution
        cameraResolution = command[1] & 0x0f;
        cameraFarmat = (command[1] & 0x70) >> 4;
        takePicture(cam, (CAM_IMAGE_MODE)cameraResolution, (CAM_IMAGE_PIX_FMT)cameraFarmat);
        break;
    case SET_VIDEO_RESOLUTION: // Set Video Resolution
        cameraResolution = command[1] & 0x0f;
        state = startPreview(cam, (CAM_VIDEO_MODE)cameraResolution);
        if (state == CAM_ERR_NO_CALLBACK)
        {
            am_util_stdio_printf("callback function is not registered\n");
        }

        message.command = CAMERA_COMMAND_STREAM_START;
        camera_task_send(&message);
        break;
    case SET_BRIGHTNESS: // Set brightness
        setBrightness(cam, (CAM_BRIGHTNESS_LEVEL)command[1]);
        break;
    case SET_CONTRAST: // Set Contrast
        setContrast(cam, (CAM_CONTRAST_LEVEL)command[1]);
        break;
    case SET_SATURATION: // Set saturation
        setSaturation(cam, (CAM_STAURATION_LEVEL)command[1]);
        break;
    case SET_EV: // Set EV
        setEV(cam, (CAM_EV_LEVEL)command[1]);
        break;
    case SET_WHITEBALANCE: // Set White balance
        setAutoWhiteBalanceMode(cam, (CAM_WHITE_BALANCE)command[1]);
        break;
    case SET_SPECIAL_EFFECTS: // Set Special effects
        setColorEffect(cam, (CAM_COLOR_FX)command[1]);
        break;
    case SET_FOCUS_CONTROL: // Focus Control
        setAutoFocus(cam, command[1]);
        if (command[1] == 0)
        {
            setAutoFocus(cam, 0x02);
        }
        break;
    case SET_EXPOSUREANDGAIN_CONTROL: // exposure and  Gain control
        setAutoExposure(cam, command[1] & 0x01);
        setAutoISOSensitive(cam, command[1] & 0x01);
        break;
    case SET_WHILEBALANCE_CONTROL: // while balance control
        setAutoWhiteBalance(cam, command[1] & 0x01);
        break;
    case SET_SHARPNESS:
        setSharpness(cam, (CAM_SHARPNESS_LEVEL)command[1]);
        break;
    case SET_MANUAL_GAIN: // manual gain control
        gainValue = (command[1] << 8) | command[2];
        setISOSensitivity(cam, gainValue);
        break;
    case SET_MANUAL_EXPOSURE: // manual exposure control
        exposureLen1 = command[1];
        exposureLen2 = command[2];
        exposureLen3 = command[3];
        exposureValue = (exposureLen1 << 16) | (exposureLen2 << 8) | exposureLen3;
        setAbsoluteExposure(cam, exposureValue);
        break;
    case GET_CAMERA_INFO: // Get Camera info
        reportCameraInfo(cam);
        break;
    case TAKE_PICTURE:
        takePicture(cam, (CAM_IMAGE_MODE)cameraResolution, (CAM_IMAGE_PIX_FMT)cameraFarmat);
        cameraGetPicture(cam);
        break;
    case DEBUG_WRITE_REGISTER:
        debugWriteRegister(cam, command + 1);
        break;
    case STOP_STREAM:
        stopPreview(cam);
        break;
    case GET_FRM_VER_INFO: // Get Firmware version info
        reportVerInfo(cam);
        break;
    case GET_SDK_VER_INFO: // Get sdk version info
        reportSdkVerInfo(cam);
        break;
    case RESET_CAMERA:
        reset(cam);
        break;
    case SET_IMAGE_QUALITY:
        setImageQuality(cam, (IMAGE_QUALITY)command[1]);
        break;
    }
    return CAM_ERR_SUCCESS;
}

static void camera_timer_callback(TimerHandle_t timer)
{
    camera_message_t message;
    message.command = CAMERA_COMMAND_STREAM_REFRESH;
    camera_task_send(&message);
}

static void camera_process_host_command(uint8_t ch)
{
    command_buffer[command_length] = ch;
    command_length++;
    if (ch == 0xAA)
    {
        camera_process_command(&camera, &command_buffer[1]);
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
    camera_message_t message;
    message.command = CAMERA_COMMAND_STREAM_STOP;
    camera_task_send(&message);

    camera_stream_read = 0;
    camera_stream_started = 0;
    uint32_t len = 9;

    arducamUartWrite(0xff);
    arducamUartWrite(0xBB);
    arducamUartWrite(0xff);
    arducamUartWrite(0xAA);
    arducamUartWrite(0x06);
    arducamUartWriteBuff((uint8_t *)&len, 4);
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
    camera_message_t message;

    camera_setup();
    while (1)
    {
        if (xQueueReceive(camera_queue_handle, &message, portMAX_DELAY) == pdPASS)
        {
            switch (message.command)
            {
            case CAMERA_COMMAND_STREAM_START:
                xTimerStart(camera_timer_handle, portMAX_DELAY);
                break;

            case CAMERA_COMMAND_STREAM_STOP:
                xTimerStop(camera_timer_handle, portMAX_DELAY);
                break;

            case CAMERA_COMMAND_STREAM_REFRESH:
                captureThread(&camera);
                break;

            default:
                break;
            }
        }
    }
}

void camera_task_create(uint32_t priority)
{
    camera_queue_handle = xQueueCreate(10, sizeof(camera_message_t));
    camera_timer_handle = xTimerCreate("camera timer", 20, pdTRUE, NULL, camera_timer_callback);
    xTaskCreate(camera_task, "camera", 512, 0, priority, &camera_task_handle);
}

void camera_task_send(camera_message_t *message)
{
    if (camera_queue_handle)
    {
        BaseType_t xHigherPriorityTaskWoken;

        if (xPortIsInsideInterrupt() == pdTRUE)
        {
            xHigherPriorityTaskWoken = pdFALSE;
            xQueueSendFromISR(camera_queue_handle, message, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else
        {
            xQueueSend(camera_queue_handle, message, portMAX_DELAY);
        }
    }
}