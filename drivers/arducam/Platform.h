/*
 * This file is part of the Arducam SPI Camera project.
 *
 * Copyright 2021 Arducam Technology co., Ltd. All Rights Reserved.
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 */
#ifndef __PLATFORM_H
#define __PLATFORM_H

extern void arducamSpiBegin(void);
extern uint8_t arducamSpiTransfer(uint8_t val);
extern void arducamSpiTransferBlock(uint8_t *data, uint16_t len);
extern void arducamSpiCsPinHigh(int pin);
extern void arducamSpiCsPinLow(int pin);
extern void arducamCsOutputMode(int pin);
extern void arducamDelayMs(uint16_t val);
extern void arducamDelayUs(uint16_t val);

#endif /*__PLATFORM_H*/