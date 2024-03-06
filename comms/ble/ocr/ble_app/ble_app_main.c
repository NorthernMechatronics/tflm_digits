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

#include <string.h>

#include <am_mcu_apollo.h>

#include "wsf_types.h"

#include "app_api.h"
#include "app_db.h"
#include "app_hw.h"
#include "app_ui.h"
#include "att_api.h"
#include "bstream.h"
#include "dm_api.h"
#include "gatt/gatt_api.h"
#include "hci_api.h"
#include "svc_ch.h"
#include "svc_core.h"
#include "svc_dis.h"
#include "wsf_msg.h"
#include "wsf_trace.h"

#include "bas_api.h"
#include "svc_batt.h"
#include "ocr_api.h"
#include "svc_ocr.h"

#include "ble.h"
#include "ble_app_api.h"

#define BLE_APP_MSG_START   0xA0

enum
{
    OCR_INFERENCE_COMPLETE_IND = BLE_APP_MSG_START,
};

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! Application message type */
typedef union
{
    wsfMsgHdr_t hdr;
    dmEvt_t dm;
    attsCccEvt_t ccc;
    attEvt_t att;
} BleAppMsg_t;

/**************************************************************************************************
  Configurable Parameters
**************************************************************************************************/

/*! configurable parameters for advertising */
static const appAdvCfg_t bleAppAdvCfg = {
    {0, 0, 0},    /*! Advertising durations in ms */
    {800, 800, 0} /*! Advertising intervals in 0.625 ms units */
};

/*! configurable parameters for slave */
static const appSlaveCfg_t bleAppSlaveCfg = {
    BLE_APP_CONN_MAX, /*! Maximum connections */
};

/*! configurable parameters for security */
static const appSecCfg_t bleAppSecCfg = {
    DM_AUTH_BOND_FLAG, /*! Authentication and bonding flags */
    0,                 /*! Initiator key distribution flags */
    DM_KEY_DIST_LTK,   /*! Responder key distribution flags */
    FALSE,             /*! TRUE if Out-of-band pairing data is present */
    FALSE              /*! TRUE to initiate security upon connection */
};

/*! configurable parameters for AMOTA connection parameter update */
static appUpdateCfg_t bleAppUpdateCfg = {
    0,   /*! Connection idle period in ms before attempting
                                              connection parameter update; set to zero to disable */
    640, /*! 800ms. Minimum connection interval in 1.25ms units */
    800, /*! 1000ms. Minimum connection interval in 1.25ms units */
    0,   /*! Connection latency */
    900, /*! 9000ms. Supervision timeout in 10ms units */
    5    /*! Number of update attempts before giving up */
};

enum
{
    BLE_APP_GATT_SC_CCC_IDX,
    BLE_APP_BATT_LVL_CCC_IDX,
    BLE_APP_OCR_CCC_IDX,
    BLE_APP_NUM_CCC_IDX
};

static const attsCccSet_t bleAppCccSet[BLE_APP_NUM_CCC_IDX] = {
    {GATT_SC_CH_CCC_HDL, ATT_CLIENT_CFG_INDICATE, DM_SEC_LEVEL_NONE},
    {BATT_LVL_CH_CCC_HDL, ATT_CLIENT_CFG_NOTIFY, DM_SEC_LEVEL_NONE},
    {OCR_CH_CCC_HDL, ATT_CLIENT_CFG_NOTIFY, DM_SEC_LEVEL_NONE}};

static basCfg_t bleAppBasCfg = {60, 1, 30};

/**************************************************************************************************
  Advertising Data
**************************************************************************************************/

/*! advertising data, discoverable mode */
static const uint8_t bleAppAdvDataDisc[] = {
    /*! flags */
    2,                        /*! length */
    DM_ADV_TYPE_FLAGS,        /*! AD type */
    DM_FLAG_LE_GENERAL_DISC | /*! flags */
    DM_FLAG_LE_BREDR_NOT_SUP,

    /*! tx power */
    2,                    /*! length */
    DM_ADV_TYPE_TX_POWER, /*! AD type */
    0,                    /*! tx power */

    /*! service UUID list */
    3,                   /*! length */
    DM_ADV_TYPE_16_UUID, /*! AD type */
    UINT16_TO_BYTES(ATT_UUID_DEVICE_INFO_SERVICE)};

/*! scan data, discoverable mode */
static const uint8_t bleAppScanDataDisc[] = {
    /*! device name */
    4,                      /*! length */
    DM_ADV_TYPE_LOCAL_NAME, /*! AD type */
    'N',
    'M',
    'I'};

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! WSF handler ID */
static wsfHandlerId_t bleAppHandlerId;

/*************************************************************************************************/
/*!
 *  \fn     BleAppDmCback
 *
 *  \brief  Application DM callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void BleAppDmCback(dmEvt_t *pDmEvt)
{
    dmEvt_t *pMsg;
    uint16_t len;

    len = DmSizeOfEvt(pDmEvt);
    if ((pMsg = WsfMsgAlloc(len)) != NULL)
    {
        memcpy(pMsg, pDmEvt, len);
        WsfMsgSend(bleAppHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     BleAppAttCback
 *
 *  \brief  Application ATT callback.
 *
 *  \param  pEvt    ATT callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void BleAppAttCback(attEvt_t *pEvt)
{
    attEvt_t *pMsg;

    if ((pMsg = WsfMsgAlloc(sizeof(attEvt_t) + pEvt->valueLen)) != NULL)
    {
        memcpy(pMsg, pEvt, sizeof(attEvt_t));
        pMsg->pValue = (uint8_t *)(pMsg + 1);
        memcpy(pMsg->pValue, pEvt->pValue, pEvt->valueLen);
        WsfMsgSend(bleAppHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     BleAppCccCback
 *
 *  \brief  Application ATTS client characteristic configuration callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void BleAppCccCback(attsCccEvt_t *pEvt)
{
    attsCccEvt_t *pMsg;
    appDbHdl_t dbHdl;

    /* if CCC not set from initialization and there's a device record */
    if ((pEvt->handle != ATT_HANDLE_NONE) &&
        ((dbHdl = AppDbGetHdl((dmConnId_t)pEvt->hdr.param)) != APP_DB_HDL_NONE))
    {
        /* store value in device database */
        AppDbSetCccTblValue(dbHdl, pEvt->idx, pEvt->value);
    }

    if ((pMsg = WsfMsgAlloc(sizeof(attsCccEvt_t))) != NULL)
    {
        memcpy(pMsg, pEvt, sizeof(attsCccEvt_t));
        WsfMsgSend(bleAppHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     BleAppProcCccState
 *
 *  \brief  Process CCC state change.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void BleAppProcCccState(BleAppMsg_t *pMsg)
{
    if (pMsg->ccc.idx == BLE_APP_OCR_CCC_IDX)
    {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY)
        {
            OcrEnableNotification((dmConnId_t)pMsg->ccc.hdr.param, BLE_APP_OCR_CCC_IDX);
        }
        else
        {
            OcrDisableNotification();
        }
    }
}

/*************************************************************************************************/
/*!
 *  \fn     BleAppClose
 *
 *  \brief  Perform UI actions on connection close.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void BleAppClose(BleAppMsg_t *pMsg)
{
}

/*************************************************************************************************/
/*!
 *  \fn     BleAppSetup
 *
 *  \brief  Set up advertising and other procedures that need to be performed after
 *          device reset.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void BleAppSetup(BleAppMsg_t *pMsg)
{
    /* set advertising and scan response data for discoverable mode */
    AppAdvSetData(
        APP_ADV_DATA_DISCOVERABLE, sizeof(bleAppAdvDataDisc), (uint8_t *)bleAppAdvDataDisc);
    AppAdvSetData(
        APP_SCAN_DATA_DISCOVERABLE, sizeof(bleAppScanDataDisc), (uint8_t *)bleAppScanDataDisc);

    /* set advertising and scan response data for connectable mode */
    AppAdvSetData(APP_ADV_DATA_CONNECTABLE, 0, NULL);
    AppAdvSetData(APP_SCAN_DATA_CONNECTABLE, 0, NULL);

    /* start advertising; automatically set connectable/discoverable mode and bondable mode */
    AppAdvStart(APP_MODE_AUTO_INIT);
}

/*************************************************************************************************/
/*!
 *  \fn     BleAppBtnCback
 *
 *  \brief  Button press callback.
 *
 *  \param  btn    Button press.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void BleAppBtnCback(uint8_t btn)
{
    dmConnId_t connId;

    /* button actions when connected */
    if ((connId = AppConnIsOpen()) != DM_CONN_ID_NONE)
    {
        switch (btn)
        {
        case APP_UI_BTN_1_LONG:
            AppConnClose(connId);
            break;
        default:
            break;
        }
    }
    /* button actions when not connected */
    else
    {
        switch (btn)
        {
        case APP_UI_BTN_1_SHORT:
            /* start or restart advertising */
            AppAdvStart(APP_MODE_AUTO_INIT);
            break;

        case APP_UI_BTN_1_MED:
            /* enter discoverable and bondable mode mode */
            AppSetBondable(TRUE);
            AppAdvStart(APP_MODE_DISCOVERABLE);
            break;

        case APP_UI_BTN_1_LONG:
            /* clear bonded device info and restart advertising */
            AppDbDeleteAllRecords();
            AppAdvStart(APP_MODE_AUTO_INIT);
            break;

        default:
            break;
        }
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Discovery callback.
 *
 *  \param  connId    Connection identifier.
 *  \param  status    Service or configuration status.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void BleAppDiscCback(dmConnId_t connId, uint8_t status)
{
    switch (status)
    {
    case APP_DISC_INIT:
        break;

    case APP_DISC_READ_DATABASE_HASH:
        /* Read peer's database hash */
        AppDiscReadDatabaseHash(connId);
        break;

    case APP_DISC_SEC_REQUIRED:
        /* request security */
        AppSlaveSecurityReq(connId);
        break;

    case APP_DISC_START:
        /* initialize discovery state */
        break;

    case APP_DISC_FAILED:
       /* Else falls through. */
    case APP_DISC_CMPL:
        /* next discovery state */
        break;

    case APP_DISC_CFG_START:
        /* start configuration */
        break;

    case APP_DISC_CFG_CMPL:
        AppDiscComplete(connId, APP_DISC_CFG_CMPL);
        break;

    case APP_DISC_CFG_CONN_START:
        /* no connection setup configuration for this application */
        break;

    default:
        break;
    }
}

static void BleAppProcMsg(BleAppMsg_t *pMsg)
{
    uint8_t uiEvent = APP_UI_NONE;

    switch (pMsg->hdr.event)
    {
    case OCR_INFERENCE_COMPLETE_IND:
        OcrNotifyInferenceResult();
        break;

    case ATTS_HANDLE_VALUE_CNF:
        break;

    case ATTS_CCC_STATE_IND:
        BleAppProcCccState(pMsg);
        break;

    case ATT_MTU_UPDATE_IND:
        APP_TRACE_INFO1("Negotiated MTU %d", ((attEvt_t *)pMsg)->mtu);
        break;

    case DM_RESET_CMPL_IND:
        AttsCalculateDbHash();
        DmSecGenerateEccKeyReq();
        BleAppSetup(pMsg);
        uiEvent = APP_UI_RESET_CMPL;
        break;

    case DM_ADV_SET_START_IND:
        uiEvent = APP_UI_ADV_SET_START_IND;
        break;

    case DM_ADV_SET_STOP_IND:
        uiEvent = APP_UI_ADV_SET_STOP_IND;
        break;

    case DM_ADV_START_IND:
        uiEvent = APP_UI_ADV_START;
        break;

    case DM_ADV_STOP_IND:
        uiEvent = APP_UI_ADV_STOP;
        break;

    case DM_CONN_OPEN_IND:
        uiEvent = APP_UI_CONN_OPEN;
        break;

    case DM_CONN_CLOSE_IND:
    {
        hciDisconnectCmplEvt_t *evt = (hciDisconnectCmplEvt_t *)pMsg;
        uiEvent = APP_UI_CONN_CLOSE;
        APP_TRACE_INFO1(">>> Connection closed reason 0x%x <<<", evt->reason);
        BleAppClose(pMsg);
    }
    break;

    case DM_CONN_UPDATE_IND:
        break;

    case DM_SEC_PAIR_CMPL_IND:
        DmSecGenerateEccKeyReq();
        uiEvent = APP_UI_SEC_PAIR_CMPL;
        break;

    case DM_SEC_PAIR_FAIL_IND:
        DmSecGenerateEccKeyReq();
        uiEvent = APP_UI_SEC_PAIR_FAIL;
        break;

    case DM_SEC_ENCRYPT_IND:
        uiEvent = APP_UI_SEC_ENCRYPT;
        break;

    case DM_SEC_ENCRYPT_FAIL_IND:
        uiEvent = APP_UI_SEC_ENCRYPT_FAIL;
        break;

    case DM_SEC_AUTH_REQ_IND:
        AppHandlePasskey(&pMsg->dm.authReq);
        break;

    case DM_SEC_ECC_KEY_IND:
        DmSecSetEccKey(&pMsg->dm.eccMsg.data.key);
        break;

    case DM_SEC_COMPARE_IND:
        AppHandleNumericComparison(&pMsg->dm.cnfInd);
        break;

    case DM_PRIV_CLEAR_RES_LIST_IND:
        APP_TRACE_INFO1("Clear resolving list status 0x%02x", pMsg->hdr.status);
        break;

    case DM_HW_ERROR_IND:
        uiEvent = APP_UI_HW_ERROR;
        break;
    default:
        break;
    }

    if (uiEvent != APP_UI_NONE)
    {
        AppUiAction(uiEvent);
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Start sensor application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BleAppStart(void)
{
    DmRegister(BleAppDmCback);
    DmConnRegister(DM_CLIENT_ID_APP, BleAppDmCback);
    AttRegister(BleAppAttCback);
    AttConnRegister(AppServerConnCback);
    AttsCccRegister(BLE_APP_NUM_CCC_IDX, (attsCccSet_t *)bleAppCccSet, BleAppCccCback);

    /* Register for app framework callbacks */
    AppUiBtnRegister(BleAppBtnCback);

    /* Register for app framework discovery callbacks */
    AppDiscRegister(BleAppDiscCback);

    /* Initialize attribute server database */
    SvcCoreGattCbackRegister(GattReadCback, GattWriteCback);
    SvcCoreAddGroup();
    SvcDisAddGroup();

    SvcBattCbackRegister(BasReadCback, NULL);
    SvcBattAddGroup();

    OcrStart();

    GattSetSvcChangedIdx(BLE_APP_GATT_SC_CCC_IDX);

    DmDevReset();
}

/*************************************************************************************************/
/*!
 *  \brief  WSF event handler for application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BleAppHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
    if (pMsg != NULL)
    {
        /* process ATT messages */
        if (pMsg->event >= ATT_CBACK_START && pMsg->event <= ATT_CBACK_END)
        {
            AppDiscProcAttMsg((attEvt_t *)pMsg);
            AppServerProcAttMsg(pMsg);
        }
        /* process DM messages */
        else if (pMsg->event >= DM_CBACK_START && pMsg->event <= DM_CBACK_END)
        {
            /* process advertising and connection-related messages */
            AppSlaveProcDmMsg((dmEvt_t *)pMsg);

            /* process security-related messages */
            AppSlaveSecProcDmMsg((dmEvt_t *)pMsg);

            /* process discovery-related messages */
            AppDiscProcDmMsg((dmEvt_t *)pMsg);
        }

        BleAppProcMsg((BleAppMsg_t *)pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Application handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BleAppHandlerInit(wsfHandlerId_t handlerId)
{
    /* store handler ID */
    bleAppHandlerId = handlerId;

    /* Set configuration pointers */
    pAppAdvCfg = (appAdvCfg_t *)&bleAppAdvCfg;
    pAppSlaveCfg = (appSlaveCfg_t *)&bleAppSlaveCfg;
    pAppSecCfg = (appSecCfg_t *)&bleAppSecCfg;
    pAppUpdateCfg = (appUpdateCfg_t *)&bleAppUpdateCfg;

    /* Initialize application framework */
    AppSlaveInit();
    AppServerInit();

    BasInit(handlerId, (basCfg_t *)&bleAppBasCfg);
}

void ble_inference_result_notify()
{
    if (!OcrIsNotificationStarted())
    {
        return;
    }

    BleAppMsg_t *msg = WsfMsgAlloc(sizeof(BleAppMsg_t));
    if (msg)
    {
        msg->hdr.event = OCR_INFERENCE_COMPLETE_IND;
        WsfMsgSend(bleAppHandlerId, msg);
    }
}
