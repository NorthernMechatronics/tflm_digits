/*************************************************************************************************/
/*!
 *  \file   hci_tr.c
 *
 *  \brief  HCI transport module.
 *
 *  Copyright (c) 2011-2018 Arm Ltd. All Rights Reserved.
 *
 *  Copyright (c) 2019 Packetcraft, Inc.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
/*************************************************************************************************/

#include <string.h>
#include "wsf_types.h"
#include "wsf_msg.h"
#include "util/bstream.h"
#include "hci_api.h"
#include "hci_core.h"
#include "hci_tr.h"

#include "hci_drv.h"
#include "wsf_trace.h"

/*************************************************************************************************/
/*!
 *  \fn     hciTrSendAclData
 *        
 *  \brief  Send a complete HCI ACL packet to the transport. 
 *
 *  \param  pContext Connection context.
 *  \param  pData    WSF msg buffer containing an ACL packet.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrSendAclData(void *pContext, uint8_t *pData)
{
  uint16_t len;

  /* get 16-bit length */
  BYTES_TO_UINT16(len, (pData + 2));
  len += HCI_ACL_HDR_LEN;

  /* transmit ACL header and data */
  if (hciDrvWrite(HCI_ACL_TYPE, len, pData) == len)
  {
      /* dump event for protocol analysis */
      HCI_PDUMP_TX_ACL(len, pData);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Send a complete HCI command to the transport.
 *
 *  \param  pCmdData    WSF msg buffer containing an HCI command.
 *
 *  \return None.
 */
/*************************************************************************************************/
void hciTrSendCmd(uint8_t *pCmdData)
{
  uint16_t   len;  // in case like LE set periodic advertising data, the maximum HCI command parameter length is 255

  /* get length */
  len = pCmdData[2] + HCI_CMD_HDR_LEN;

  /* transmit ACL header and data */
  if (hciDrvWrite(HCI_CMD_TYPE, len, pCmdData) == len)
  {
      /* dump event for protocol analysis */
      HCI_PDUMP_CMD(len, pCmdData);
  }
}