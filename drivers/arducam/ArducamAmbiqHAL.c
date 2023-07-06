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

#include <am_bsp.h>

#include <FreeRTOS.h>
#include <task.h>

#include "Platform.h"
#include "ArducamUart.h"

#define ARDUCAM_SPI_IOM (0)
static void *arducam_spi_handle;
static am_hal_iom_config_t arducam_spi_config;

static TaskHandle_t arducam_task_handle;
static void arducam_task(void *parameter);

#define UART_BUFFER_SIZE    (128)
static uint8_t uart_buffer[UART_BUFFER_SIZE];
static uint32_t uart_read_length;
static uint32_t uart_data_length;
static am_hal_uart_transfer_t uart_transfer = {
    .ui32Direction = AM_HAL_UART_READ,
    .pui8Data = uart_buffer,
    .ui32NumBytes = UART_BUFFER_SIZE,
    .ui32TimeoutMs = 0,
    .pui32BytesTransferred = 0,
};

void arducamSpiBegin(void)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCK,  g_AM_BSP_GPIO_IOM0_SCK);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MISO, g_AM_BSP_GPIO_IOM0_MISO);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MOSI, g_AM_BSP_GPIO_IOM0_MOSI);

//  Arducam driver uses manual chip select control
//    am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS,   g_AM_BSP_GPIO_IOM0_CS);
    arducamCsOutputMode(AM_BSP_GPIO_IOM0_CS);

    arducam_spi_config.eInterfaceMode = AM_HAL_IOM_SPI_MODE;
    arducam_spi_config.ui32ClockFreq = AM_HAL_IOM_4MHZ;
    arducam_spi_config.eSpiMode = AM_HAL_IOM_SPI_MODE_0;

    am_hal_iom_initialize(ARDUCAM_SPI_IOM, &arducam_spi_handle);
    am_hal_iom_power_ctrl(arducam_spi_handle, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_iom_configure(arducam_spi_handle, &arducam_spi_config);
    am_hal_iom_enable(arducam_spi_handle);
}

uint8_t arducamSpiTransfer(uint8_t val)
{
    uint32_t data = 0;

    if (val == 0)
    {
        am_hal_iom_transfer_t rx;

        rx.ui32InstrLen                = 0;
        rx.ui32Instr                   = 0;
        rx.eDirection                  = AM_HAL_IOM_RX;
        rx.ui32NumBytes                = 1;
        rx.pui32RxBuffer               = (uint32_t *)&data;
        rx.bContinue                   = false;
        rx.ui8RepeatCount              = 0;
        rx.ui32PauseCondition          = 0;
        rx.ui32StatusSetClr            = 0;
        rx.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

        am_hal_iom_blocking_transfer(arducam_spi_handle, &rx);
    }
    else
    {
        am_hal_iom_transfer_t tx;

        data = val;

        tx.ui32InstrLen                = 0;
        tx.ui32Instr                   = 0;
        tx.eDirection                  = AM_HAL_IOM_TX;
        tx.ui32NumBytes                = 1;
        tx.pui32TxBuffer               = (uint32_t *)&data;
        tx.bContinue                   = false;
        tx.ui8RepeatCount              = 0;
        tx.ui32PauseCondition          = 0;
        tx.ui32StatusSetClr            = 0;
        tx.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

        am_hal_iom_blocking_transfer(arducam_spi_handle, &tx);
    }

    return data;
}

void arducamSpiTransferBlock(uint8_t *data, uint16_t len)
{
        am_hal_iom_transfer_t rx;

        rx.ui32InstrLen                = 0;
        rx.ui32Instr                   = 0;
        rx.eDirection                  = AM_HAL_IOM_RX;
        rx.ui32NumBytes                = len;
        rx.pui32RxBuffer               = (uint32_t *)data;
        rx.bContinue                   = false;
        rx.ui8RepeatCount              = 0;
        rx.ui32PauseCondition          = 0;
        rx.ui32StatusSetClr            = 0;
        rx.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

        am_hal_iom_blocking_transfer(arducam_spi_handle, &rx);
 }

void arducamSpiCsPinHigh(int pin)
{
    am_hal_gpio_state_write(AM_BSP_GPIO_IOM0_CS, AM_HAL_GPIO_OUTPUT_SET);
}

void arducamSpiCsPinLow(int pin)
{
    am_hal_gpio_state_write(AM_BSP_GPIO_IOM0_CS, AM_HAL_GPIO_OUTPUT_SET);
}

void arducamCsOutputMode(int pin)
{
    am_hal_gpio_pinconfig(pin,   g_AM_HAL_GPIO_OUTPUT);
}

void arducamDelayMs(uint16_t val)
{
    am_util_delay_ms(val);
}

void arducamDelayUs(uint16_t val)
{
    am_util_delay_us(val);
}

void SerialBegin(uint32_t baudRate)
{
    am_bsp_buffered_uart_printf_enable();
    NVIC_SetPriority((IRQn_Type)(UART0_IRQn + AM_BSP_UART_PRINT_INST),
                    NVIC_configKERNEL_INTERRUPT_PRIORITY);
    memset(uart_buffer, 0, UART_BUFFER_SIZE);
    uart_data_length = 0;
    uart_read_length = 0;
    xTaskCreate(arducam_task, "arducam", 512, 0, 2, &arducam_task_handle);
}

void SerialWrite(uint8_t ch)
{
    am_bsp_uart_send(&ch, 1);
}

void SerialWriteBuff(uint8_t *buf, uint32_t len)
{
    am_bsp_uart_send(buf, len);
}

void SerialPrintf(const char *str)
{
    am_bsp_uart_string_print((char *)str);
}

uint8_t SerialRead()
{
    uint8_t rt = 0;
    rt = uart_buffer[uart_read_length];
    uart_read_length++;

    if (uart_read_length == uart_data_length)
    {
        uart_read_length = 0;
        uart_data_length = 0;
    }

    return rt;
}

static void arducam_task(void *parameter)
{
    uint32_t ui32BytesRead = 0;

    while (1)
    {
        xTaskNotifyWait(0, 1, NULL, portMAX_DELAY);

        uart_transfer.pui32BytesTransferred = &ui32BytesRead;
        am_bsp_com_uart_transfer(&uart_transfer);
        if (ui32BytesRead > 0)
        {
            uart_data_length = ui32BytesRead;
        }
    }
}

void am_uart_isr()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    am_bsp_buffered_uart_service();
    xTaskNotifyFromISR(arducam_task_handle, 0, eNoAction, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}