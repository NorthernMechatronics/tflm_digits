#ifndef __ARDUCAM_SLOT_H
#define __ARDUCAM_SLOT_H

#include <stdint.h>

void SerialBegin(uint32_t baudRate);
void SerialWrite(uint8_t ch);
void SerialWriteBuff(uint8_t *buf, uint32_t len);
void SerialPrintf(const char *str);
uint8_t SerialRead();

#endif