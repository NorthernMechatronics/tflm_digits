#include <stdint.h>

#include <am_mcu_apollo.h>
#include <am_util.h>
#include <am_bsp.h>

#include "ArducamLink.h"
#include "ArducamUart.h"

#include "string.h"

void SerialBegin(uint32_t baudRate)
{
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
}

uint8_t SerialRead()
{
}

uint32_t SerialAvailable()
{
}

void delayUs(uint32_t val)
{
    am_util_delay_us(val);
}


