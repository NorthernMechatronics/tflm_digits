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

void *camera_iom_handle;

void camera_delay_ms(uint32_t delay)
{
    am_util_delay_ms(delay);
}

void camera_wake()
{
    am_hal_iom_config_t config;
    config.eInterfaceMode = AM_HAL_IOM_SPI_MODE;
    config.ui32ClockFreq = AM_HAL_IOM_1MHZ;
    config.eSpiMode = AM_HAL_IOM_SPI_MODE_0;

// FIXME:
//
// On the Petal development board, the entire I/Os can be
// powered on and off for power savings and current draw 
// measurement.  In the development version, we need to
// explicitly enable the I/O header pins.  This will be
// resolved in the production version where it will default
// to enable.
#if defined(BSP_NM180410)
    am_hal_gpio_pinconfig(30, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(30, AM_HAL_GPIO_OUTPUT_SET);
#endif

    am_bsp_iom_pins_enable(0, AM_HAL_IOM_SPI_MODE);
    am_hal_iom_initialize(0, &camera_iom_handle);
    am_hal_iom_power_ctrl(camera_iom_handle, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_iom_configure(camera_iom_handle, &config);
    am_hal_iom_enable(camera_iom_handle);
}

void camera_sleep()
{
    am_hal_iom_disable(camera_iom_handle);
    am_hal_iom_power_ctrl(camera_iom_handle, AM_HAL_SYSCTRL_DEEPSLEEP, false);
    am_bsp_iom_pins_disable(0, AM_HAL_IOM_SPI_MODE);
}

uint32_t camera_reg_read(uint8_t address, uint8_t *value, size_t length, bool persist)
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

    uint32_t status = am_hal_iom_blocking_transfer(camera_iom_handle, &transfer);

    return status;
}

uint32_t camera_reg_write(uint8_t address, uint8_t *value, size_t length, bool persist)
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

    uint32_t status = am_hal_iom_blocking_transfer(camera_iom_handle, &transfer);
    return status;
}

uint32_t camera_buf_read(uint8_t *value, size_t length, bool persist)
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

    uint32_t status = am_hal_iom_blocking_transfer(camera_iom_handle, &transfer);

    return status;
}

