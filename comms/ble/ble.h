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
#ifndef _BLE_H_
#define _BLE_H_

/**
 * @brief BLE stack state
 * 
 */
typedef enum
{
    BLE_STACK_STARTED,
    BLE_STACK_STOPPED,
} ble_stack_state_e;

/**
 * @brief Set the BLE stack state.
 * 
 * @param eState  Only BLE_STACK_STARTED is valid
 */
extern void ble_stack_state_set(ble_stack_state_e eState);

/**
 * @brief Retrieve the BLE stack state.
 * 
 * @param peState 
 */
extern void ble_stack_state_get(ble_stack_state_e *peState);

/**
 * @brief Start BLE advertisement.
 * 
 */
extern void ble_advertise_start();

/**
 * @brief Stop BLE advertisement.
 * 
 */
extern void ble_advertise_stop();

/**
 * @brief Enable/disable debug messages printing.
 * 
 * @param ui32Enabled 
 */
extern void ble_tracing_set(uint32_t ui32Enabled);

#endif