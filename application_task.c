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

#include <FreeRTOS.h>
#include <task.h>

#include "am_bsp.h"

#ifdef RAT_LORAWAN_ENABLE
#include "lorawan.h"
#endif

#ifdef RAT_BLE_ENABLE
#include "ble.h"
#endif

#include "application_task.h"
#include "application_task_cli.h"

#define APPLICATION_DEFAULT_LORAWAN_CLASS   LORAWAN_CLASS_A

static TaskHandle_t application_task_handle;

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
}

#ifdef RAT_LORAWAN_ENABLE

static void application_on_lorawan_sleep()
{
    am_hal_gpio_state_write(AM_BSP_GPIO_LED2, AM_HAL_GPIO_OUTPUT_CLEAR);
}

static void application_on_lorawan_wake()
{
    am_hal_gpio_state_write(AM_BSP_GPIO_LED2, AM_HAL_GPIO_OUTPUT_SET);
}

static void application_on_join_request(LmHandlerJoinParams_t *params)
{
    if (params->Status == LORAMAC_HANDLER_ERROR)
    {
        lorawan_join();
    }
    else
    {
        lorawan_class_set(APPLICATION_DEFAULT_LORAWAN_CLASS);
        lorawan_request_time_sync();
    }
}

static void application_on_receive(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
    // appData is NULL for beacon messages
    if (appData)
    {
        if (appData->Port > 0)
        {
            am_util_stdio_printf("\n\rReceived Data\n\r");
            am_util_stdio_printf("COUNTER   : %-4d\n\r", params->DownlinkCounter);
            am_util_stdio_printf("PORT      : %-4d\n\r", appData->Port);
            am_util_stdio_printf("SLOT      : %-4d\n\r", params->RxSlot);
            am_util_stdio_printf("DATA RATE : %-4d\n\r", params->Datarate);
            am_util_stdio_printf("RSSI      : %-4d\n\r", params->Rssi);
            am_util_stdio_printf("SNR       : %-4d\n\r", params->Snr);
            am_util_stdio_printf("SIZE      : %-4d\n\r", appData->BufferSize);
            am_util_stdio_printf("PAYLOAD   :\n\r");
            for (int i = 0; i < appData->BufferSize; i++)
            {
                am_util_stdio_printf("%02x ", appData->Buffer[i]);
            }
            am_util_stdio_printf("\n\r");
        }
    }
}

static void application_setup_lorawan()
{
    lorawan_tracing_set(1);

    lorawan_network_config(
        LORAWAN_REGION_US915,
        LORAWAN_DATARATE_0,
        true,
        true);

    lorawan_activation_config(LORAWAN_ACTIVATION_OTAA, NULL);

    lorawan_key_set_by_str(LORAWAN_KEY_JOIN_EUI, "b4c231a359bc2e3d");
    lorawan_key_set_by_str(LORAWAN_KEY_APP, "01c3f004a2d6efffe32c4eda14bcd2b4");
    lorawan_key_set_by_str(LORAWAN_KEY_NWK, "3f4ca100e2fc675ea123f4eb12c4a012");

    lorawan_event_callback_register(LORAWAN_EVENT_RX_DATA, application_on_receive);
    lorawan_event_callback_register(LORAWAN_EVENT_JOIN_REQUEST, application_on_join_request);

    lorawan_event_callback_register(LORAWAN_EVENT_SLEEP, application_on_lorawan_sleep);
    lorawan_event_callback_register(LORAWAN_EVENT_WAKE, application_on_lorawan_wake);

    // start the LoRaWAN stack
    lorawan_stack_state_set(LORAWAN_STACK_STARTED);

    if (lorawan_get_join_state())
    {
        lorawan_class_set(APPLICATION_DEFAULT_LORAWAN_CLASS);
    }
}

#endif

#ifdef RAT_BLE_ENABLE

static void application_setup_ble()
{
    ble_tracing_set(1);

    ble_stack_state_set(BLE_STACK_STARTED);
}

#endif

static void application_task(void *parameter)
{
    application_task_cli_register();

    application_setup_task();
#ifdef RAT_LORAWAN_ENABLE
    application_setup_lorawan();
#endif
    
#ifdef RAT_BLE_ENABLE
    application_setup_ble();
#endif

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        am_hal_gpio_state_write(AM_BSP_GPIO_LED0, AM_HAL_GPIO_OUTPUT_TOGGLE);
    }
}

void application_task_create(uint32_t priority)
{
    xTaskCreate(application_task, "application", 512, 0, priority, &application_task_handle);
}