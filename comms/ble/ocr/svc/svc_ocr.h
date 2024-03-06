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

#ifndef SVC_OCR_H
#define SVC_OCR_H

#include "att_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*! \addtogroup OCR_SERVICE
 *  \{ */

/** \name OCR Service UUID: 00000005-fa73-4979-965f-5f7b8f26157a
 */
#define ATT_UUID_OCR_SERVICE \
  0x7A, 0x15, 0x26, 0x8F, 0x7B, 0x5F, \
  0x5F, 0x96, \
  0x79, 0x49, \
  0x73, 0xFA, \
  0x05, 0x00, 0x00, 0x00

/** \name OCR Characteristic UUID: 00000006-fa73-4979-965f-5f7b8f26157a
 */
#define ATT_UUID_OCR_CH \
  0x7A, 0x15, 0x26, 0x8F, 0x7B, 0x5F, \
  0x5F, 0x96, \
  0x79, 0x49, \
  0x73, 0xFA, \
  0x06, 0x00, 0x00, 0x00

#define OCR_MAX_SIZE  (16 + ATT_VALUE_NTF_LEN)

/**************************************************************************************************
 Handle Ranges
**************************************************************************************************/

/** \name Message Service Handles
 *
 */
/**@{*/
#define OCR_START_HDL 0x50              /*!< \brief Start handle. */
#define OCR_END_HDL   (OCR_MAX_HDL - 1) /*!< \brief End handle. */

/**************************************************************************************************
 Handles
**************************************************************************************************/

/*! \brief Service Handles */
enum
{
    OCR_SVC_HDL = OCR_START_HDL, /*!< \brief OCR Message service declaration */
    OCR_CH_HDL,              /*!< \brief OCR Message string characteristic */
    OCR_HDL,                 /*!< \brief OCR Message string */
    OCR_CH_CCC_HDL,          /*!< \brief OCR Message string CCCD */
    OCR_CH_UD_HDL,           /*!< \brief OCR Message string user description */
    OCR_MAX_HDL                  /*!< \brief Maximum handle. */
};
/**@}*/

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Add the services to the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcOcrAddGroup(void);

/*************************************************************************************************/
/*!
 *  \brief  Remove the services from the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcOcrRemoveGroup(void);

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
void SvcOcrCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback);

/*! \} */ /* MESSAGE_SERVICE */

#ifdef __cplusplus
};
#endif

#endif