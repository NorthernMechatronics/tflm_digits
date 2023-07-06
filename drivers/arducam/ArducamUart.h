#ifndef __ARDUCAM_SLOT_H
#define __ARDUCAM_SLOT_H

#include <stdint.h>
#include <am_util.h>

#define printf am_util_stdio_printf
#define sprintf am_util_stdio_sprintf

void SerialBegin(uint32_t baudRate);
void SerialWrite(uint8_t ch);
void SerialWriteBuff(uint8_t *buf, uint32_t len);
void SerialPrintf(const char *str);
uint8_t SerialRead();
uint32_t SerialAvailable();
void delayUs(uint32_t val);

#endif