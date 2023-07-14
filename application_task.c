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

#include "arducam_mega.h"

#include "application_task.h"
#include "application_task_cli.h"

#include "tflm.h"

static TaskHandle_t application_task_handle;

#define CAMERA_BLOCK_READ_SIZE (128)

static void *iom_handle;
static uint8_t image[CAMERA_BLOCK_READ_SIZE];

static void camera_wake()
{
    am_hal_iom_config_t config;
    config.eInterfaceMode = AM_HAL_IOM_SPI_MODE;
    config.ui32ClockFreq = AM_HAL_IOM_1MHZ;
    config.eSpiMode = AM_HAL_IOM_SPI_MODE_0;

    am_bsp_iom_pins_enable(0, AM_HAL_IOM_SPI_MODE);
    am_hal_iom_initialize(0, &iom_handle);
    am_hal_iom_power_ctrl(iom_handle, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_iom_configure(iom_handle, &config);
    am_hal_iom_enable(iom_handle);
}

static void camera_sleep()
{
    am_hal_iom_disable(iom_handle);
    am_hal_iom_power_ctrl(iom_handle, AM_HAL_SYSCTRL_DEEPSLEEP, false);
    am_bsp_iom_pins_disable(0, AM_HAL_IOM_SPI_MODE);
}

static uint32_t camera_reg_read(uint8_t address, uint8_t *value, size_t length, bool persist)
{
    am_hal_iom_transfer_t transfer;

    uint32_t instr = address & 0x7F;

    transfer.ui32InstrLen = 1;
    transfer.ui32Instr = instr;
    transfer.eDirection = AM_HAL_IOM_RX;
    transfer.ui32NumBytes = length;
    transfer.pui32RxBuffer = (uint32_t *)value;
    transfer.bContinue = persist;
    transfer.ui8RepeatCount = 0;
    transfer.ui32PauseCondition = 0;
    transfer.ui32StatusSetClr = 0;
    transfer.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

    uint32_t status = am_hal_iom_blocking_transfer(iom_handle, &transfer);

    return status;
}

static uint32_t camera_reg_write(uint8_t address, uint8_t *value, size_t length, bool persist)
{
    am_hal_iom_transfer_t transfer;

    uint32_t instr = address | 0x80;

    transfer.ui32InstrLen = 1;
    transfer.ui32Instr = instr;
    transfer.eDirection = AM_HAL_IOM_TX;
    transfer.ui32NumBytes = length;
    transfer.pui32TxBuffer = (uint32_t *)value;
    transfer.bContinue = persist;
    transfer.ui8RepeatCount = 0;
    transfer.ui32PauseCondition = 0;
    transfer.ui32StatusSetClr = 0;
    transfer.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

    uint32_t status = am_hal_iom_blocking_transfer(iom_handle, &transfer);
    return status;
}

static uint32_t camera_buf_read(uint8_t *value, size_t length, bool persist)
{
    am_hal_iom_transfer_t transfer;

    transfer.ui32InstrLen = 0;
    transfer.ui32Instr = 0;
    transfer.eDirection = AM_HAL_IOM_RX;
    transfer.ui32NumBytes = length;
    transfer.pui32RxBuffer = (uint32_t *)value;
    transfer.bContinue = persist;
    transfer.ui8RepeatCount = 0;
    transfer.ui32PauseCondition = 0;
    transfer.ui32StatusSetClr = 0;
    transfer.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

    uint32_t status = am_hal_iom_blocking_transfer(iom_handle, &transfer);

    return status;
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

    arducam_mega_interface_t interface;
    interface.wake = camera_wake;
    interface.sleep = camera_sleep;
    interface.reg_read = camera_reg_read;
    interface.reg_write = camera_reg_write;
    interface.buf_read = camera_buf_read;
    interface.delay_ms = am_util_delay_ms;

    arducam_mega_setup(&interface);
    arducam_mega_wake();
}

static void application_task(void *parameter)
{
    application_task_cli_register();

    application_setup_task();

    memset(image, 0, CAMERA_BLOCK_READ_SIZE);
    arducam_mega_capture_config(0x0A, 0x02);
    arducam_mega_capture_blocking();
    uint32_t picture_size = arducam_mega_capture_length();
    uint32_t block_size = CAMERA_BLOCK_READ_SIZE;
    while (1)
    {
        if (picture_size > 0)
        {
            if (picture_size <= CAMERA_BLOCK_READ_SIZE)
            {
                arducam_mega_capture_read(image, picture_size);
                for (int i = 0; i < block_size; i++)
                {
                    am_util_stdio_printf("0x%02x ", image[i]);
                    if (i % 16 == 0)
                    {
                        am_util_stdio_printf("\r\n");
                    }
                }
                picture_size = 0;
                am_util_stdio_printf("Capture completed\r\n");
            }
            else
            {
                arducam_mega_capture_read(image, block_size);
                for (int i = 0; i < block_size; i++)
                {
                    am_util_stdio_printf("0x%02x ", image[i]);
                    if (i % 16 == 0)
                    {
                        am_util_stdio_printf("\r\n");
                    }
                }

                picture_size -= CAMERA_BLOCK_READ_SIZE;
            }
            vTaskDelay(20);
            am_hal_gpio_state_write(AM_BSP_GPIO_LED0, AM_HAL_GPIO_OUTPUT_TOGGLE);
        }
        else
        {
            vTaskDelay(500);
            am_hal_gpio_state_write(AM_BSP_GPIO_LED0, AM_HAL_GPIO_OUTPUT_TOGGLE);
        }
    }
}

void application_task_create(uint32_t priority)
{
    xTaskCreate(application_task, "application", 512, 0, priority, &application_task_handle);
}