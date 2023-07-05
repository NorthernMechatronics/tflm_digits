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
#ifndef _LORAWAN_CONFIG_H_
#define _LORAWAN_CONFIG_H_

#include <LmHandler.h>

#define LORAWAN_EEPROM_NUMBER_OF_PAGES    (2)
#define LORAWAN_EEPROM_START_ADDRESS      (AM_HAL_FLASH_INSTANCE_SIZE - (LORAWAN_EEPROM_NUMBER_OF_PAGES * AM_HAL_FLASH_PAGE_SIZE))

#define LORAWAN_CLOCK_SOURCE    AM_HAL_STIMER_XTAL_32KHZ
#define LORAWAN_CLOCK_PERIOD    32768

// The maximum payload size assuming FOpts field is not used is 242 bytes.
// Each data fragment message consists of a 2 byte index.  Hence, each
// fragment has an actual data block size of 240 bytes.
#define LORAWAN_FRAGMENT_MAX_SIZE       (240)

// The maximum number of fragments assuming a smallest fragment size
// of 50 bytes.  This allows an image size of:
//    409,600 bytes at the lowest data rate
//  1,966,080 bytes at the highest data rate
#define LORAWAN_FRAGMENT_MAX_NUMBER    (8192)

// The redundancy number selected is based on the acceptable frame
// error rate in 3G and LTE since the LoRaWAN spec does
// not specify an acceptable error rate.  For 8192 number of fragments
// and an error rate of 1.25%, we require at least:
//
//      8192 * 0.0125 = 102.4 or 103 redundancy fragments
// 
// We will use 160 for redundancy.
#define LORAWAN_FRAGMENT_MAX_REDUNDANCY  (160)

#endif