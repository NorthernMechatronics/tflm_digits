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

#include "arducam_mega.h"

#define ARDUCHIP_FRAMES    0x01
#define ARDUCHIP_TEST1     0x00 // TEST register
#define ARDUCHIP_FIFO      0x04 // FIFO and I2C control
#define ARDUCHIP_FIFO_2    0x07 // FIFO and I2C control
#define FIFO_CLEAR_ID_MASK 0x01
#define FIFO_START_MASK    0x02

#define FIFO_RDPTR_RST_MASK 0x10
#define FIFO_WRPTR_RST_MASK 0x20
#define FIFO_CLEAR_MASK     0x80

#define ARDUCHIP_TRIG 0x44 // Trigger source
#define VSYNC_MASK    0x01
#define SHUTTER_MASK  0x02
#define CAP_DONE_MASK 0x04

#define FIFO_SIZE1 0x45       // Camera write FIFO size[7:0] for burst to read
#define FIFO_SIZE2 0x46       // Camera write FIFO size[15:8]
#define FIFO_SIZE3 0x47       // Camera write FIFO size[18:16]

#define BURST_FIFO_READ  0x3C // Burst FIFO read operation
#define SINGLE_FIFO_READ 0x3D // Single FIFO read operation

#if defined(__MSP430G2553__)
#define PREVIEW_BUF_LEN 50
#else
#define PREVIEW_BUF_LEN 255
#endif

#define CAPRURE_MAX_NUM 0xff

#define CAM_REG_POWER_CONTROL                      0X02
#define CAM_REG_SENSOR_RESET                       0X07
#define CAM_REG_FORMAT                             0X20
#define CAM_REG_CAPTURE_RESOLUTION                 0X21
#define CAM_REG_BRIGHTNESS_CONTROL                 0X22
#define CAM_REG_CONTRAST_CONTROL                   0X23
#define CAM_REG_SATURATION_CONTROL                 0X24
#define CAM_REG_EV_CONTROL                         0X25
#define CAM_REG_WHILEBALANCE_MODE_CONTROL          0X26
#define CAM_REG_COLOR_EFFECT_CONTROL               0X27
#define CAM_REG_SHARPNESS_CONTROL                  0X28
#define CAM_REG_AUTO_FOCUS_CONTROL                 0X29
#define CAM_REG_IMAGE_QUALITY                      0x2A
#define CAM_REG_EXPOSURE_GAIN_WHILEBALANCE_CONTROL 0X30
#define CAM_REG_MANUAL_GAIN_BIT_9_8                0X31
#define CAM_REG_MANUAL_GAIN_BIT_7_0                0X32
#define CAM_REG_MANUAL_EXPOSURE_BIT_19_16          0X33
#define CAM_REG_MANUAL_EXPOSURE_BIT_15_8           0X34
#define CAM_REG_MANUAL_EXPOSURE_BIT_7_0            0X35
#define CAM_REG_BURST_FIFO_READ_OPERATION          0X3C
#define CAM_REG_SINGLE_FIFO_READ_OPERATION         0X3D
#define CAM_REG_SENSOR_ID                          0x40
#define CAM_REG_YEAR_ID                            0x41
#define CAM_REG_MONTH_ID                           0x42
#define CAM_REG_DAY_ID                             0x43
#define CAM_REG_SENSOR_STATE                       0x44
#define CAM_REG_FPGA_VERSION_NUMBER                0x49
#define CAM_REG_DEBUG_DEVICE_ADDRESS               0X0A
#define CAM_REG_DEBUG_REGISTER_HIGH                0X0B
#define CAM_REG_DEBUG_REGISTER_LOW                 0X0C
#define CAM_REG_DEBUG_REGISTER_VALUE               0X0D

#define CAM_REG_SENSOR_STATE_IDLE (1 << 1)
#define CAM_SENSOR_RESET_ENABLE   (1 << 6)
#define CAM_FORMAT_BASICS         (0 << 0)
#define CAM_SET_CAPTURE_MODE      (0 << 7)
#define CAM_SET_VIDEO_MODE        (1 << 7)

#define SET_WHILEBALANCE 0X02
#define SET_EXPOSURE     0X01
#define SET_GAIN         0X00

#define CAMERA_TYPE_NUMBER 2

// #define FORMAT_NONE                                0X00
// #define FORMAT_JPEG                                0X01
// #define FORMAT_RGB                                 0X02
// #define FORMAT_YUV                                 0X03

#define RESOLUTION_160X120   (1 << 0)
#define RESOLUTION_320X240   (1 << 1)
#define RESOLUTION_640X480   (1 << 2)
#define RESOLUTION_800X600   (1 << 3)
#define RESOLUTION_1280X720  (1 << 4)
#define RESOLUTION_1280X960  (1 << 5)
#define RESOLUTION_1600X1200 (1 << 6)
#define RESOLUTION_1920X1080 (1 << 7)
#define RESOLUTION_2048X1536 (1 << 8)
#define RESOLUTION_2592X1944 (1 << 9)
#define RESOLUTION_320x320   (1 << 10)
#define RESOLUTION_128x128   (1 << 11)
#define RESOLUTION_96x96     (1 << 12)

#define SPECIAL_NORMAL       (0 << 0)
#define SPECIAL_BLUEISH      (1 << 0)
#define SPECIAL_REDISH       (1 << 1)
#define SPECIAL_BW           (1 << 2)
#define SPECIAL_SEPIA        (1 << 3)
#define SPECIAL_NEGATIVE     (1 << 4)
#define SPECIAL_GREENISH     (1 << 5)
#define SPECIAL_OVEREXPOSURE (1 << 6)
#define SPECIAL_SOLARIZE     (1 << 7)
#define SPECIAL_YELLOWISH    (1 << 8)

static arducam_mega_interface_t arducam_mega_interface;
static arducam_mega_info_t arducam_mega_info;
static arducam_mega_info_t camera_info = {
    .model = "3MP",
    .resolutions = RESOLUTION_320x320 | RESOLUTION_128x128 | RESOLUTION_96x96 | RESOLUTION_320X240 |
                   RESOLUTION_640X480 | RESOLUTION_1280X720 | RESOLUTION_1600X1200 |
                   RESOLUTION_1920X1080 | RESOLUTION_2048X1536,
    .effects = SPECIAL_BLUEISH | SPECIAL_REDISH | SPECIAL_BW | SPECIAL_SEPIA | SPECIAL_NEGATIVE |
               SPECIAL_GREENISH | SPECIAL_YELLOWISH,
    .exposureMax = 30000,
    .exposureMin = 1,
    .gainMax = 1023,
    .gainMin = 1,
    .focus = 0,
    .sharpness = 1,
    .address = 0x78,
    .id = 0,
    .year = 0,
    .month = 0,
    .day = 0,
    .version = 0
};

static uint32_t arducam_mega_capture_started;

static void arducam_mega_delay_ms(uint32_t value)
{
    if (arducam_mega_interface.delay_ms)
    {
        arducam_mega_interface.delay_ms(value);
    }
}

static void arducam_mega_wait()
{
    uint8_t status[2];
    arducam_mega_interface.reg_read(CAM_REG_SENSOR_STATE, status, 2, false);
    while ((status[1] & 0x03) != CAM_REG_SENSOR_STATE_IDLE)
    {
        arducam_mega_delay_ms(2);
        arducam_mega_interface.reg_read(CAM_REG_SENSOR_STATE, status, 2, false);
    }
}

static void arducam_mega_reg_read(uint8_t address, uint8_t *value)
{
    uint8_t data[2];

    arducam_mega_interface.reg_read(address, data, 2, false);
    *value = data[1];
}

static void arducam_mega_reg_write(uint8_t address, uint8_t value)
{
    arducam_mega_interface.reg_write(address, &value, 1, false);
}

static uint8_t arducam_mega_read_bit(uint8_t address, uint8_t mask)
{
    uint8_t value;
    arducam_mega_reg_read(address, &value);
    value = value & mask;
    return value;
}

static void arducam_mega_fifo_clear_flag(void)
{
    arducam_mega_reg_write(ARDUCHIP_FIFO, FIFO_CLEAR_ID_MASK);
}

static void arducam_mega_fifo_flush(void)
{
    arducam_mega_reg_write(ARDUCHIP_FIFO_2, FIFO_CLEAR_MASK);
}

static uint32_t arducam_mega_fifo_get_length(void)
{
    uint8_t l1, l2, l3;
    uint32_t length;
    arducam_mega_reg_read(FIFO_SIZE1, &l1);
    arducam_mega_reg_read(FIFO_SIZE2, &l2);
    arducam_mega_reg_read(FIFO_SIZE3, &l3);

    length = ((l3 << 16) | (l2 << 8) | l1) & 0xFFFFFF;

    return length;
}

static void arducam_mega_fifo_read_byte(uint8_t *value)
{
    arducam_mega_reg_read(SINGLE_FIFO_READ, value);
}

void arducam_mega_setup(arducam_mega_interface_t *interface)
{
    memcpy(&arducam_mega_interface, interface, sizeof(arducam_mega_interface_t));
    memset(&arducam_mega_info, 0, sizeof(arducam_mega_info_t));
}

void arducam_mega_wake(void)
{
    arducam_mega_interface.wake();

    arducam_mega_reg_write(CAM_REG_SENSOR_RESET, CAM_SENSOR_RESET_ENABLE);
    arducam_mega_wait();
    arducam_mega_reg_read(CAM_REG_SENSOR_ID, &arducam_mega_info.id);
    arducam_mega_wait();
    arducam_mega_reg_read(CAM_REG_YEAR_ID, &arducam_mega_info.year);
    arducam_mega_wait();
    arducam_mega_reg_read(CAM_REG_MONTH_ID, &arducam_mega_info.month);
    arducam_mega_wait();
    arducam_mega_reg_read(CAM_REG_DAY_ID, &arducam_mega_info.day);
    arducam_mega_wait();
    arducam_mega_reg_read(CAM_REG_FPGA_VERSION_NUMBER, &arducam_mega_info.version);
    arducam_mega_wait();
    arducam_mega_reg_write(CAM_REG_DEBUG_DEVICE_ADDRESS, 0x78);
    arducam_mega_wait();

    arducam_mega_capture_started = 0;
}

void arducam_mega_sleep(void)
{
    arducam_mega_interface.sleep();
}

uint32_t arducam_mega_ready(void)
{
    uint8_t value[2];
    arducam_mega_interface.reg_read(CAM_REG_SENSOR_STATE, value, 2, false);
    return ((value[1] & 0x03) == CAM_REG_SENSOR_STATE_IDLE) ? 1 : 0;
}

void arducam_mega_capture_config(uint8_t mode, uint8_t format)
{
    arducam_mega_reg_write(CAM_REG_FORMAT, format);
    arducam_mega_wait();
    arducam_mega_reg_write(CAM_REG_CAPTURE_RESOLUTION, CAM_SET_CAPTURE_MODE | mode);
    arducam_mega_wait();
}

void arducam_mega_capture_blocking()
{
    arducam_mega_fifo_clear_flag();
    arducam_mega_reg_write(ARDUCHIP_FIFO, FIFO_START_MASK);
    while (!arducam_mega_capture_done())
    {
        arducam_mega_delay_ms(1);
    }
    arducam_mega_capture_started = 1;
}

uint32_t arducam_mega_capture_done()
{
    return arducam_mega_read_bit(ARDUCHIP_TRIG, CAP_DONE_MASK);
}

uint32_t arducam_mega_capture_length()
{
    return arducam_mega_fifo_get_length();
}

void arducam_mega_capture_read(uint8_t *buffer, size_t size)
{
    uint8_t data;

    if (arducam_mega_capture_started)
    {
        arducam_mega_capture_started = 0;
        arducam_mega_interface.reg_read(BURST_FIFO_READ, &data, 1, true);
        arducam_mega_interface.buf_read(buffer, size, false);
        /*
        for (int i = 0; i < size - 1; i++)
        {
            arducam_mega_interface.buf_read(data, 1, true);
            buffer[i] = data[0];
        }
        arducam_mega_interface.buf_read(data, 1, false);
        buffer[size - 1] = data[0];
        */
    }
    else
    {
        arducam_mega_interface.reg_read(BURST_FIFO_READ, buffer, size, false);
    }
}