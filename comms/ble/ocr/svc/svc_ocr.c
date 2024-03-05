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

#include "wsf_types.h"
#include "att_api.h"
#include "wsf_trace.h"
#include "util/bstream.h"
#include "svc_cfg.h"
#include "svc_ocr.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! Characteristic read permissions */
#ifndef OCR_SEC_PERMIT_READ
#define OCR_SEC_PERMIT_READ SVC_SEC_PERMIT_READ
#endif

/*! Characteristic write permissions */
#ifndef OCR_SEC_PERMIT_WRITE
#define OCR_SEC_PERMIT_WRITE SVC_SEC_PERMIT_WRITE
#endif

/**************************************************************************************************
 Static Variables
**************************************************************************************************/

/**************************************************************************************************
 Service variables
**************************************************************************************************/

/* Service Declaration */
static const uint8_t ocrValSvc[] = { ATT_UUID_OCR_SERVICE };
static const uint8_t ocrLenSvc = sizeof(ocrValSvc);

/* Characteristic */
static const uint8_t ocrValStrCh[] = {ATT_PROP_READ | ATT_PROP_WRITE, UINT16_TO_BYTES(OCR_HDL), ATT_UUID_OCR_CH };
static const uint16_t ocrLenStrCh = sizeof(ocrValStrCh);

/* String Value */
static const uint8_t svcOcrStrUuid[ATT_128_UUID_LEN] = { ATT_UUID_OCR_CH };
static uint8_t ocrValStr[OCR_MAX_SIZE] = { 0 };
static const uint16_t ocrLenStr = sizeof(ocrValStr);

/* Client Characteristic Configuration Descriptor */
static uint8_t ocrValStrChCcc[] = { UINT16_TO_BYTES(0x0000) };
static const uint16_t ocrLenStrChCcc = sizeof(ocrValStrChCcc);

/* User Description */
static const uint8_t ocrValStrChUd[] = "OCR Inference Control";
static const uint16_t ocrLenStrChUd = sizeof(ocrValStrChUd) - 1u;

/* Attribute list */
static const attsAttr_t ocrList[] =
{
/* Service Declaration */
  {
    attPrimSvcUuid,
    (uint8_t *) ocrValSvc,
    (uint16_t *) &ocrLenSvc,
    sizeof(ocrValSvc),
    0,
    ATTS_PERMIT_READ
  },
  /* Characteristic Declaration */
  {
    attChUuid,
    (uint8_t *) ocrValStrCh,
    (uint16_t *) &ocrLenStrCh,
    sizeof(ocrValStrCh),
    0,
    ATTS_PERMIT_READ
  },
  /* String Value */
  {
    svcOcrStrUuid,
    ocrValStr,
    (uint16_t *) &ocrLenStr,
    sizeof(ocrValStr),
    ATTS_SET_WRITE_CBACK | ATTS_SET_VARIABLE_LEN,
    OCR_SEC_PERMIT_READ | OCR_SEC_PERMIT_WRITE
  },
  /* Client Characteristic Configuration Descriptor */
  {
    attCliChCfgUuid,
    ocrValStrChCcc,
    (uint16_t *) &ocrLenStrChCcc,
    sizeof(ocrValStrChCcc),
    ATTS_SET_CCC,
    OCR_SEC_PERMIT_READ | OCR_SEC_PERMIT_WRITE
  },
  {
    attChUserDescUuid,
    (uint8_t *)ocrValStrChUd,
    (uint16_t *) &ocrLenStrChUd,
    sizeof(ocrValStrChUd) - 1,
    0,
    OCR_SEC_PERMIT_READ
  }
};

/* Group structure */
static attsGroup_t svcOcrGroup =
{
  NULL,
  (attsAttr_t *) ocrList,
  NULL,
  NULL,
  OCR_START_HDL,
  OCR_END_HDL
};

/*************************************************************************************************/
/*!
 *  \brief  Add the services to the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcOcrAddGroup(void)
{
  /* add services */
  AttsAddGroup(&svcOcrGroup);
}

/*************************************************************************************************/
/*!
 *  \brief  Remove the services from the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcOcrRemoveGroup(void)
{
  AttsRemoveGroup(OCR_START_HDL);
}

/*************************************************************************************************/
/*!
 *  \brief  Register callbacks for the service.
 *
 *  \param  readCback   Read callback function.
 *  \param  writeCback  Write callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcOcrCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback)
{
  svcOcrGroup.readCback = readCback;
  svcOcrGroup.writeCback = writeCback;
}