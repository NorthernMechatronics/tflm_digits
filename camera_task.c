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
#include "camera_task_cli.h"
#include "console_task.h"

#define COMMAND_BUFFER_LEN (64)

static ArducamCamera camera;
static uint8_t command_buffer[COMMAND_BUFFER_LEN];
static uint8_t command_length;
static uint8_t sendFlag = TRUE;
static uint32_t camera_stream_read = 0;
static uint8_t camera_stream_started = 0;

static uint8_t r_max, g_max, b_max;

static TaskHandle_t camera_task_handle;
static QueueHandle_t camera_queue_handle;
static TimerHandle_t camera_timer_handle;

#define IMAGE_PROCESS_BLOCK_SIZE (192)
#define IMAGE_WIDTH  (32)
#define IMAGE_HEIGHT (32)
#define IMAGE_CHANNEL (3)
#define IMAGE_SIZE  (IMAGE_WIDTH * IMAGE_HEIGHT * IMAGE_CHANNEL)

static uint8_t image_process_buffer[IMAGE_PROCESS_BLOCK_SIZE];
static uint8_t image_rgb888[IMAGE_SIZE];
static uint8_t image_row_index, image_column_index;
static uint32_t image_process_index = 0;
static uint32_t image_capture_state = 0;

typedef struct camera_event_callback_s
{
    camera_command_t event;
    camera_event_handler_t handler;
} camera_event_callback_t;

static camera_event_callback_t camera_event_callback[CAMERA_COMMAND_MAXLEN];

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
        message.command = CAMERA_COMMAND_STREAM_STOP;
        camera_task_send(&message);
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

static void camera_retrieve_still(void)
{
    if (camera.receivedLength > 0)
    {
        // process only one block at a time to avoid blocking other tasks
        uint32_t data_length = readBuff(&camera, image_process_buffer, IMAGE_PROCESS_BLOCK_SIZE);

        uint32_t i = 0;
        while (i < data_length)
        {
            if ((image_row_index % 3) == 0)
            {
                if ((i % 3) == 0)
                {
                    uint8_t r_raw = (image_process_buffer[i] & 0b11111000) >> 3;
                    uint8_t b_raw = (image_process_buffer[i+1] & 0b00011111);
                    uint8_t g_upper = (image_process_buffer[i] & 0b00000111) << 3;
                    uint8_t g_lower = (image_process_buffer[i+1] & 0b11100000) >> 5;
                    uint8_t g_raw = g_upper | g_lower;
                    i += 6;
                    image_rgb888[image_process_index++] = r_raw;
                    image_rgb888[image_process_index++] = g_raw;
                    image_rgb888[image_process_index++] = b_raw;

                    if (r_raw > r_max)
                    {
                        r_max = r_raw;
                    }

                    if (g_raw > g_max)
                    {
                        g_max = g_raw;
                    }

                    if (b_raw > b_max)
                    {
                        b_max = b_raw;
                    }
                }
            }
            else
            {
                break;
            }
        }
        image_row_index++;

        if (camera.receivedLength > 0)
        {
            camera_message_t message;
            message.command = CAMERA_COMMAND_STILL_RETRIEVE;
            camera_task_send(&message);
        }
        else
        {
            camera_message_t message;
            message.command = CAMERA_COMMAND_STILL_RETRIEVE_DONE;
            camera_task_send(&message);
        }
    }
}

static void camera_print_capture(void)
{
    am_util_stdio_printf("\r\n\r\n");
    am_util_stdio_printf("Captured Image:\r\n");
    for (int i = 0; i < IMAGE_SIZE; i += 3)
    {
        am_util_stdio_printf("0x%02x 0x%02x 0x%02x\r\n", image_rgb888[i], image_rgb888[i+1], image_rgb888[i+2]);
    }
     am_util_stdio_printf("\r\n\r\n");
}

static void camera_normalize()
{
    uint32_t r, g, b, gray;
    for (int i = 0; i < IMAGE_SIZE; i+=3)
    {
        r = image_rgb888[i] * 127 / r_max;
        image_rgb888[i] = r;

        g = image_rgb888[i+1] * 127 / g_max;
        image_rgb888[i+1] = g;

        b = image_rgb888[i+2] * 127 / b_max;
        image_rgb888[i+2] = b;
    }
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
    reset(&camera);
    takePicture(&camera, 10, 2);
    image_capture_state = 0;
}

static void camera_task(void *parameter)
{
    camera_message_t message;

    camera_task_cli_register();
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

            case CAMERA_COMMAND_STILL_CAPTURE:
                image_process_index = 0;
                image_row_index = 0;
                image_column_index = 0;
                takePicture(&camera,
                    (CAM_IMAGE_MODE)message.payload.capture_parameters.resolution,
                    (CAM_IMAGE_PIX_FMT)message.payload.capture_parameters.format);
                memset(image_rgb888, 0, IMAGE_SIZE);
                if (image_capture_state < 2)
                {
                    image_capture_state++;
                    message.command = CAMERA_COMMAND_STILL_CAPTURE;
                    camera_task_send(&message);
                }
                else
                {
                    image_capture_state = 0;
                    r_max = b_max = g_max = 0;
                    camera_retrieve_still();
                }
                break;

            case CAMERA_COMMAND_STILL_RETRIEVE:
                camera_retrieve_still();
                break;

            case CAMERA_COMMAND_STILL_RETRIEVE_DONE:
                image_capture_state = 0;
                camera_normalize();
                if (camera_event_callback[CAMERA_COMMAND_STILL_RETRIEVE_DONE].handler)
                {
                    camera_event_callback[CAMERA_COMMAND_STILL_RETRIEVE_DONE].handler(image_rgb888, IMAGE_SIZE);
                }
                else
                {
                    am_util_stdio_printf("No callback attached, displaying raw capture:\r\n");
                    camera_print_capture();
                }
                break;

            default:
                break;
            }
        }
    }
}

void camera_task_create(uint32_t priority)
{
    memset(camera_event_callback, 0, sizeof(camera_event_callback));
    camera_queue_handle = xQueueCreate(10, sizeof(camera_message_t));
    camera_timer_handle = xTimerCreate("camera timer", 50, pdTRUE, NULL, camera_timer_callback);
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

void camera_event_subscribe(camera_command_t event, camera_event_handler_t handler)
{
    for (size_t i = 0; i < CAMERA_COMMAND_MAXLEN; i++)
    {
        if (event == i)
        {
            camera_event_callback[i].handler = handler;
            return;
        }
    }
}
