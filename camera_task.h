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
#ifndef _CAMERA_TASK_H_
#define _CAMERA_TASK_H_

#include <stdint.h>

typedef enum camera_command_e {
    CAMERA_COMMAND_STREAM_START,
    CAMERA_COMMAND_STREAM_STOP,
    CAMERA_COMMAND_STREAM_REFRESH,
    CAMERA_COMMAND_STILL_CAPTURE,
    CAMERA_COMMAND_STILL_RETRIEVE,
    CAMERA_COMMAND_STILL_RETRIEVE_DONE,
    CAMERA_COMMAND_MAXLEN
} camera_command_t;

typedef struct camera_capture_parameters_s
{
    uint16_t resolution;
    uint16_t format;
} camera_capture_parameters_t;

typedef union camera_message_payload_u
{
    uint8_t *buffer;
    camera_capture_parameters_t capture_parameters;
} camera_message_payload_t;

typedef struct camera_message_s
{
    camera_command_t command;
    camera_message_payload_t payload;
} camera_message_t;

typedef void (*camera_event_handler_t)(uint8_t *, size_t size);

extern void camera_task_create(uint32_t priority);
extern void camera_task_send(camera_message_t *message);
extern void camera_event_subscribe(camera_command_t event, camera_event_handler_t handler);

#endif