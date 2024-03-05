/*
 *  BSD 3-Clause License
 *
 * Copyright (c) 2024, Northern Mechatronics, Inc.
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
#include <stdint.h>
#include <string.h>

#include "svc_ocr.h"
#include "ocr_api.h"

#define OCR_INFERENCE_RESULT_SIZE  (OCR_MAX_SIZE - ATT_VALUE_NTF_LEN)

static dmConnId_t OcrConnId = DM_CONN_ID_NONE;
static uint8_t OcrCccIdx;
static ocr_handler_t OcrHandler;

static uint8_t OcrInferenceResult[OCR_INFERENCE_RESULT_SIZE];

static uint8_t ocrReadCback(dmConnId_t connId, uint16_t handle, uint8_t operation,
                            uint16_t offset, attsAttr_t *pAttr)
{
    if (handle == OCR_HDL)
    {
        size_t length = *pAttr->pLen;
        memcpy(&pAttr->pValue[offset], OcrInferenceResult, length); 
    }
    else
    {
        ATT_ERR_NOT_SUP;
    }

    return ATT_SUCCESS;
}

static uint8_t ocrWriteCback(dmConnId_t connId, uint16_t handle, uint8_t operation,
                                uint16_t offset, uint16_t len, uint8_t *pValue,
                                attsAttr_t *pAttr)
{
    if (OcrHandler)
    {
        OcrHandler();
    }

    return ATT_SUCCESS;
}

void OcrStart(void)
{
    SvcOcrCbackRegister(ocrReadCback, ocrWriteCback);
    SvcOcrAddGroup();
}

bool_t OcrIsNotificationStarted(void)
{
    return OcrConnId != DM_CONN_ID_NONE;
}

void OcrEnableNotification(dmConnId_t connId, uint8_t cccIdx)
{
    OcrConnId = OcrConnId;
    OcrCccIdx = cccIdx;
}

void OcrDisableNotification(void)
{
    OcrConnId = DM_CONN_ID_NONE;
}

void OcrRegisterInferenceTrigger(ocr_handler_t handler)
{
    OcrHandler = handler;
}

void OcrUpdateInferenceResult(uint8_t *data, size_t length)
{
    if (OcrConnId == DM_CONN_ID_NONE ||
        !AttsCccEnabled(OcrConnId, OcrCccIdx))
    {
        return;
    }

    memcpy(OcrInferenceResult, data, length);
    AttsHandleValueNtf(OcrConnId, OCR_HDL, length, OcrInferenceResult);
}