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
#include <stdlib.h>
#include <string.h>

#include <am_mcu_apollo.h>
#include <am_util.h>

#include <FreeRTOS.h>
#include <FreeRTOS_CLI.h>

#include "ota_config.h"
#include "application_task_cli.h"

static portBASE_TYPE application_task_cli_entry(char *pui8OutBuffer,
                                                size_t ui32OutBufferLength,
                                                const char *pui8Command);

static CLI_Command_Definition_t application_task_cli_definition = {
    (const char *const) "app",
    (const char *const) "app    :  Application Commands.\r\n",
    application_task_cli_entry,
    -1};

void application_task_cli_register()
{
    FreeRTOS_CLIRegisterCommand(&application_task_cli_definition);
}

static void help(char *pui8OutBuffer, size_t argc, char **argv)
{
    strcat(pui8OutBuffer, "\r\nusage: app <command>\r\n");
    strcat(pui8OutBuffer, "\r\n");
    strcat(pui8OutBuffer, "supported commands are:\r\n");
    strcat(pui8OutBuffer, "  reset  perform a soft reset\r\n");
    strcat(pui8OutBuffer, "  ota    < |set|erase> manages the OTA descriptor\r\n");
}

static void ota(char *pui8OutBuffer, size_t argc, char **argv)
{
    uint32_t *p_ota_desc = (uint32_t *)(OTA_POINTER_LOCATION & ~(AM_HAL_FLASH_PAGE_SIZE - 1));
    uint32_t i;

    if (argc == 2)
    {
        for (i = 0; i < AM_HAL_SECURE_OTA_MAX_OTA + 1; i++)
        {
            if (p_ota_desc[i] == 0xFFFFFFFF)
            {
                break;
            }
            if (((p_ota_desc[i] &  0x03) == AM_HAL_OTA_STATUS_ERROR)
             || ((p_ota_desc[i] & ~0x03) >= 0x100000))
            {
                break;
            }
        }

        if (p_ota_desc[i] == 0xFFFFFFFF)
        {
            am_hal_ota_status_t status[AM_HAL_SECURE_OTA_MAX_OTA];
            am_hal_get_ota_status(p_ota_desc, AM_HAL_SECURE_OTA_MAX_OTA, status);

            for (uint32_t n = 0; n < AM_HAL_SECURE_OTA_MAX_OTA; n++)
            {
                if ((uint32_t)status[n].pImage == 0xFFFFFFFF)
                {
                    break;
                }
                char *buffer = pui8OutBuffer + strlen(pui8OutBuffer);
                am_util_stdio_sprintf(buffer, "Previous OTA state address: 0x%08x - status: %d\r\n",
                    status[n].pImage, status[n].status);
            }
        }
        else
        {
            strcat(pui8OutBuffer, "\r\nNo previous OTA state.\r\n");
        }
        return;
    }

    if (strcmp(argv[2], "set") == 0)
    {
        p_ota_desc = (uint32_t *)(OTA_POINTER_LOCATION & ~(AM_HAL_FLASH_PAGE_SIZE - 1));
        am_hal_ota_init(AM_HAL_FLASH_PROGRAM_KEY, p_ota_desc);

        uint8_t magic = ((uint8_t *)OTA_FLASH_ADDRESS)[3];
        am_hal_ota_add(AM_HAL_FLASH_PROGRAM_KEY,
            magic,
            (uint32_t *)(OTA_FLASH_ADDRESS));
        
        strcat(pui8OutBuffer, "\r\nOTA descriptor set\r\n");
        return;
    }
    else if (strcmp(argv[2], "erase") == 0)
    {
        am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY,
            AM_HAL_FLASH_ADDR2INST(OTA_POINTER_LOCATION),
            AM_HAL_FLASH_ADDR2PAGE(OTA_POINTER_LOCATION));
        strcat(pui8OutBuffer, "\r\nOTA descriptor erased.\r\n");
    }
}

portBASE_TYPE
application_task_cli_entry(char *pui8OutBuffer, size_t ui32OutBufferLength, const char *pui8Command)
{
    size_t argc;
    char *argv[8];
    char argz[128];

    pui8OutBuffer[0] = 0;

    strcpy(argz, pui8Command);
    FreeRTOS_CLIExtractParameters(argz, &argc, argv);

    if (strcmp(argv[1], "help") == 0)
    {
        help(pui8OutBuffer, argc, argv);
    }
    else if (strcmp(argv[1], "reset") == 0)
    {
        NVIC_SystemReset();
    }
    else if (strcmp(argv[1], "ota") == 0)
    {
        ota(pui8OutBuffer, argc, argv);
    }

    return pdFALSE;
}
