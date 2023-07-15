/*
 * This file is part of the Arducam SPI Camera project.
 *
 * Copyright 2021 Arducam Technology co., Ltd. All Rights Reserved.
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 */
#include "ArducamLink.h"
#include "ArducamUart.h"

#include "string.h"

void arducamUartBegin(uint32_t baudRate)
{
    SerialBegin(baudRate);
}

void reportVerInfo(ArducamCamera* myCamera)
{
    uint8_t headAndtail[] = {0xff, 0xaa, 0x03, 0xff, 0xbb};

    uint32_t len = 6;
    arducamUartWriteBuff(&headAndtail[0], 3);
    arducamUartWriteBuff((uint8_t*)&len, 4);
    arducamUartWriteBuff(myCamera->verDateAndNumber, 4);
    printf("\r\n");
    arducamUartWriteBuff(&headAndtail[3], 2);
}

void reportSdkVerInfo(ArducamCamera* myCamera)
{
    uint8_t headAndtail[] = {0xff, 0xaa, 0x05, 0xff, 0xbb};

    uint32_t len = 6;
    arducamUartWriteBuff(&headAndtail[0], 3);
    arducamUartWriteBuff((uint8_t*)&len, 4);
    arducamUartWriteBuff((uint8_t*)&myCamera->currentSDK->sdkVersion, 4);
    printf("\r\n");
    arducamUartWriteBuff(&headAndtail[3], 2);
}

void reportCameraInfo(ArducamCamera* myCamera)
{
    uint8_t headAndtail[] = {0xff, 0xaa, 0x02, 0xff, 0xbb};

    uint32_t len = 0;
    char buff[400];
    arducamUartWriteBuff(&headAndtail[0], 3);
    sprintf(buff,
            "ReportCameraInfo\r\nCamera Type:%s\r\nCamera Support Resolution:%d\r\nCamera Support "
            "specialeffects:%d\r\nCamera Support Focus:%d\r\nCamera Exposure Value Max:%ld\r\nCamera Exposure Value "
            "Min:%d\r\nCamera Gain Value Max:%d\r\nCamera Gain Value Min:%d\r\nCamera Support Sharpness:%d\r\n",
            myCamera->myCameraInfo.cameraId, myCamera->myCameraInfo.supportResolution,
            myCamera->myCameraInfo.supportSpecialEffects, myCamera->myCameraInfo.supportFocus,
            myCamera->myCameraInfo.exposureValueMax, myCamera->myCameraInfo.exposureValueMin,
            myCamera->myCameraInfo.gainValueMax, myCamera->myCameraInfo.gainValueMin,
            myCamera->myCameraInfo.supportSharpness);
    len = strlen(buff);
    arducamUartWriteBuff((uint8_t*)&len, 4);
    printf(buff);
    arducamUartWriteBuff(&headAndtail[3], 2);
}

void cameraGetPicture(ArducamCamera* myCamera)
{
    uint8_t headAndtail[]           = {0xff, 0xaa, 0x01, 0xff, 0xbb};
    uint8_t buff[READ_IMAGE_LENGTH] = {0};

    uint8_t rtLength = 0;
    uint32_t len     = myCamera->totalLength;
    arducamUartWriteBuff(&headAndtail[0], 3);
    arducamUartWriteBuff((uint8_t*)(&len), 4);
    arducamUartWrite(((myCamera->currentPictureMode & 0x0f) << 4) | 0x01);
    while (myCamera->receivedLength) {
        rtLength = readBuff(myCamera, buff, READ_IMAGE_LENGTH);
        arducamUartWriteBuff(buff, rtLength);
    }
    arducamUartWriteBuff(&headAndtail[3], 2);
}

void send_data_pack(char cmd_type, char* msg)
{
    uint8_t headAndtail[] = {0xff, 0xaa, 0x07, 0xff, 0xbb};
    headAndtail[2]        = cmd_type;
    uint32_t len          = strlen(msg) + 2;
    arducamUartWriteBuff(&headAndtail[0], 3);
    arducamUartWriteBuff((uint8_t*)&len, 4);
    printf(msg);
    printf("\r\n");
    arducamUartWriteBuff(&headAndtail[3], 2);
}

void arducamUartWrite(uint8_t data)
{
    SerialWrite(data);
    delayUs(12);
}

void arducamUartWriteBuff(uint8_t* buff, uint16_t length)
{
    SerialWriteBuff(buff, length);
    delayUs(12);
}

uint32_t arducamUartAvailable(void)
{
    return SerialAvailable();
}

uint8_t arducamUartRead(void)
{
    return SerialRead();
}