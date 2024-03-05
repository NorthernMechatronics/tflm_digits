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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <am_bsp.h>
#include <am_mcu_apollo.h>
#include <am_util.h>

#include <FreeRTOS.h>
#include <FreeRTOS_CLI.h>
#include <queue.h>
#include <timers.h>

#include "ble.h"
#include "ble_task.h"
#include "ble_task_cli.h"
#include "console_task.h"

static portBASE_TYPE
ble_task_cli_entry(char *pui8OutBuffer, size_t ui32OutBufferLength, const char *pui8Command);

CLI_Command_Definition_t ble_task_cli_definition = {
    (const char *const) "ble",
    (const char *const) "ble    :  BLE Application Layer Commands.\r\n",
    ble_task_cli_entry,
    -1};

static size_t argc;
static char *argv[8];
static char argz[128];

void ble_task_cli_register()
{
    FreeRTOS_CLIRegisterCommand(&ble_task_cli_definition);
    argc = 0;
}

static void ble_task_cli_help(char *pui8OutBuffer, size_t argc, char **argv)
{
    strcat(pui8OutBuffer, "\r\nusage: ble <command>\r\n");
    strcat(pui8OutBuffer, "\r\n");
    strcat(pui8OutBuffer, "supported commands are:\r\n");
    strcat(pui8OutBuffer, "  start  start the BLE stack\r\n");
    strcat(pui8OutBuffer, "\r\n");
    strcat(pui8OutBuffer, "  adv    <start|stop> advertisement\r\n");
    strcat(pui8OutBuffer, "  trace  <enable|disable> debug messages\r\n");
}

static void ble_task_cli_adv(char *pui8OutBuffer, size_t argc, char **argv)
{
    if (strcmp(argv[2], "start") == 0)
    {
        ble_advertise_start();
    }
    else if (strcmp(argv[2], "stop") == 0)
    {
        ble_advertise_stop();
    }
}

static void ble_task_cli_trace(char *pui8OutBuffer, size_t argc, char **argv)
{
    if (strcmp(argv[2], "enable") == 0)
    {
        ble_tracing_set(1);
    }
    else if (strcmp(argv[2], "disable") == 0)
    {
        ble_tracing_set(0);
    }
}

static portBASE_TYPE
ble_task_cli_entry(char *pui8OutBuffer, size_t ui32OutBufferLength, const char *pui8Command)
{
    pui8OutBuffer[0] = 0;

    strcpy(argz, pui8Command);
    FreeRTOS_CLIExtractParameters(argz, &argc, argv);

    if (strcmp(argv[1], "help") == 0)
    {
        ble_task_cli_help(pui8OutBuffer, argc, argv);
    }
    else if (strcmp(argv[1], "adv") == 0)
    {
        ble_task_cli_adv(pui8OutBuffer, argc, argv);
    }
    else if (strcmp(argv[1], "trace") == 0)
    {
        ble_task_cli_trace(pui8OutBuffer, argc, argv);
    }
    else if (strcmp(argv[1], "start") == 0)
    {
        ble_stack_state_set(BLE_STACK_STARTED);
    }

    return pdFALSE;
}
