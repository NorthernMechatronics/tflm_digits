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
#ifndef _ARDUCAM_MEGA_H_
#define _ARDUCAM_MEGA_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct arducam_mega_interface_t
{
    void (*wake)(void);
    void (*sleep)(void);
    uint32_t (*reg_read)(uint8_t address, uint8_t *value, size_t size, bool persist);
    uint32_t (*reg_write)(uint8_t address, uint8_t *value, size_t size, bool persist);
    uint32_t (*buf_read)(uint8_t *value, size_t size, bool persist);
    void (*delay_ms)(uint32_t value);
} arducam_mega_interface_t;

typedef struct arducam_mega_info_t
{
    char    *model;
    uint32_t resolutions;
    uint32_t effects;
    uint32_t exposureMax;
    uint32_t exposureMin;
    uint32_t gainMax;
    uint32_t gainMin;
    uint32_t focus;
    uint32_t sharpness;
    uint32_t address;
    uint8_t id;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t version;
} arducam_mega_info_t;

void arducam_mega_setup(arducam_mega_interface_t *interface);
void arducam_mega_wake(void);
void arducam_mega_sleep(void);
uint32_t arducam_mega_ready(void);

void arducam_mega_capture_blocking();
void arducam_mega_capture_config(uint8_t mode, uint8_t format);
uint32_t arducam_mega_capture_done();
uint32_t arducam_mega_capture_length();
void arducam_mega_capture_read(uint8_t *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif