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

#include "am_bsp.h"

#include "Platform.h"
#include "ArducamUart.h"

#define ARDUCAM_SPI_IOM (0)
static void *arducam_spi_handle;
static am_hal_iom_config_t arducam_spi_config;

void arducamSpiBegin(void)
{
    am_bsp_iom_pins_enable(ARDUCAM_SPI_IOM, AM_HAL_IOM_SPI_MODE);

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
    return 0;
}

void arducamSpiTransferBlock(uint8_t *data, uint16_t len)
{
}

void arducamSpiCsPinHigh(int pin)
{
}

void arducamSpiCsPinLow(int pin)
{
}

void arducamCsOutputMode(int pin)
{
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
}

void SerialWrite(uint8_t ch)
{
}

void SerialWriteBuff(uint8_t *buf, uint32_t len)
{
}

void SerialPrintf(const char *str)
{
}

uint8_t SerialRead()
{
}
