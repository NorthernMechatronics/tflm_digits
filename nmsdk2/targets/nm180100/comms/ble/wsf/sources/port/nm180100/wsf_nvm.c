/*************************************************************************************************/
/*!
 *  \file   wsf_nvm.c
 *
 *  \brief  NVM service.
 *
 *  Copyright (c) 2019 Arm Ltd.
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
#include <am_mcu_apollo.h>
#include <string.h>

#include "ble_config.h"

#include "wsf_types.h"
#include "wsf_assert.h"
#include "wsf_nvm.h"
#include "util/crc32.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! Reserved filecode. */
#define WSF_NVM_RESERVED_FILECODE                 ((uint64_t)0)

/* Unused (erased) filecode. */
/* TODO: May depend on flash type */
#define WSF_NVM_UNUSED_FILECODE                   ((uint64_t)0xFFFFFFFFFFFFFFFF)

/*! Align value to word boundary. */
#define WSF_NVM_WORD_ALIGN(x)                     (((x) + (WSF_FLASH_WORD_SIZE - 1)) & \
                                                         ~(WSF_FLASH_WORD_SIZE - 1))

#define WSF_NVM_CRC_INIT_VALUE                    0xFEDCBA98

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! \brief      Header. */
typedef struct
{
  uint64_t          id;         /*!< Stored data ID. */
  uint32_t          len;        /*!< Stored data length. */
  uint32_t          headerCrc;  /*!< CRC of this header. */
  uint32_t          dataCrc;    /*!< CRC of subsequent data. */
} WsfNvmHeader_t;

static struct
{
  uint32_t          availAddr;  /*!< Next available address for NVM write. */
  uint32_t          sectorSize; /*!< Size of erase sector. */
  uint32_t          totalSize;  /*!< Total size of NVM storage. */
} wsfNvmCb;

/**************************************************************************************************
  Global Functions
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \brief  Initialize the WSF NVM.
 */
/*************************************************************************************************/
void WsfNvmInit(void)
{
  wsfNvmCb.totalSize = WSF_NVM_NUM_OF_PAGES * WSF_NVM_PAGE_SIZE;
  wsfNvmCb.sectorSize = WSF_NVM_PAGE_SIZE;

  WsfNvmHeader_t header;
  uint32_t storageAddr = WSF_NVM_START_ADDR;
  uint32_t headerCrc;
  bool_t corruptData = FALSE;

  do
  {
    /* Read header. */
    memcpy(&header, (const void *)storageAddr, sizeof(header));

    if (header.id == WSF_NVM_UNUSED_FILECODE)
    {
      /* Found unused entry at end of used storage. */
      break;
    }

    /* Iterate through stored data headers, looking for existing matching stored data header. */
    if (header.id != WSF_NVM_RESERVED_FILECODE)
    {
      /* Calculate CRC of header itself. */
      headerCrc = CalcCrc32(WSF_NVM_CRC_INIT_VALUE, sizeof(header.id) + sizeof(header.len),
                            (uint8_t *)&header);

      if (headerCrc != header.headerCrc)
      {
        /* Corrupt header. */
        corruptData = TRUE;
        break;
      }
    }
    else
    {
      if ((header.headerCrc != 0) || (header.dataCrc !=0))
      {
        /* Corrupt header. */
        corruptData = TRUE;
        break;
      }
    }

    /* Move to next stored data block and read header. */
    storageAddr += WSF_NVM_WORD_ALIGN(header.len) + sizeof(header);
    WSF_ASSERT((storageAddr - WSF_NVM_START_ADDR) < wsfNvmCb.totalSize);

  } while ((storageAddr - WSF_NVM_START_ADDR) < wsfNvmCb.totalSize);

  wsfNvmCb.availAddr = storageAddr;

  /* Check for corrupt data. */
  if (corruptData == TRUE)
  {
    /* Search for the first available location */
    while ((storageAddr - WSF_NVM_START_ADDR) < wsfNvmCb.totalSize)
    {
      memcpy(&header.id, (const void *)storageAddr, sizeof(header.id));

      if (header.id == WSF_NVM_UNUSED_FILECODE)
      {
        break;
      }

      storageAddr += sizeof(header.id);
    }

    /* Update the address of the first available location. align to sector boundary. */
    wsfNvmCb.availAddr = (storageAddr + wsfNvmCb.sectorSize - 1) & ~(wsfNvmCb.sectorSize - 1);

    /* Erase all data. */
    WsfNvmEraseDataAll(NULL);
  }
}

/*************************************************************************************************/
/*!
 *  \brief  Read data.
 *
 *  \param  id         Stored data ID.
 *  \param  pData      Buffer to read to.
 *  \param  len        Data length to read.
 *  \param  compCback  Read callback.
 *
 *  \return TRUE if NVM operation is successful, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t WsfNvmReadData(uint64_t id, uint8_t *pData, uint16_t len, WsfNvmCompEvent_t compCback)
{
  WsfNvmHeader_t header;
  uint32_t headerCrc, dataCrc;
  uint32_t storageAddr = WSF_NVM_START_ADDR;
  bool_t findId = FALSE;

  WSF_ASSERT(!((id == WSF_NVM_RESERVED_FILECODE) || (id == WSF_NVM_UNUSED_FILECODE)));

  /* Read first header. */
  memcpy(&header, (const void *)storageAddr, sizeof(header));

  do
  {
    if (header.id == WSF_NVM_UNUSED_FILECODE)
    {
      /* Found unused entry at end of used storage. */
      break;
    }

    /* Iterate through stored data headers, looking for existing matching stored data header. */
    if (header.id != WSF_NVM_RESERVED_FILECODE)
    {
      /* Calculate CRC of header itself. */
      headerCrc = CalcCrc32(WSF_NVM_CRC_INIT_VALUE, sizeof(header.id) + sizeof(header.len),
                            (uint8_t *)&header);

      if (headerCrc != header.headerCrc)
      {
        /* Corrupt header. */
        /* TODO: Catastrophic failure? */
        break;
      }
      else if ((header.id == id) && (header.len == len))
      {
        /* Valid header and matching ID - read data after header. */
        storageAddr += sizeof(header);
        memcpy(pData, (const void *)storageAddr, sizeof(header.len));
        dataCrc = CalcCrc32(WSF_NVM_CRC_INIT_VALUE, header.len, pData);
        if (dataCrc == header.dataCrc)
        {
          findId = TRUE;
        }
        break;
      }
    }

    /* Move to next stored data block and read header. */
    storageAddr += WSF_NVM_WORD_ALIGN(header.len) + sizeof(header);
    memcpy(&header, (const void *)storageAddr, sizeof(header));
  } while(1);

  if (compCback)
  {
    compCback(findId);
  }
  return findId;
}

/*************************************************************************************************/
/*!
 *  \brief  Write data.
 *
 *  \param  id         Stored data ID.
 *  \param  pData      Buffer to write.
 *  \param  len        Data length to write.
 *  \param  compCback  Write callback.
 *
 *  \return TRUE if NVM operation is successful, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t WsfNvmWriteData(uint64_t id, const uint8_t *pData, uint16_t len, WsfNvmCompEvent_t compCback)
{
  WsfNvmHeader_t header;
  uint32_t headerCrc, dataCrc;
  uint32_t storageAddr = WSF_NVM_START_ADDR;

  WSF_ASSERT(!((id == WSF_NVM_RESERVED_FILECODE) || (id == WSF_NVM_UNUSED_FILECODE)));
  WSF_ASSERT((wsfNvmCb.availAddr - WSF_NVM_START_ADDR) <= wsfNvmCb.totalSize);

  /* Read first header. */
  memcpy(&header, (const void *)storageAddr, sizeof(header));

  do
  {
    if (header.id == WSF_NVM_UNUSED_FILECODE)
    {
      /* Found unused entry at end of used storage. */
      break;
    }

    /* Iterate through stored data headers, looking for existing matching stored data header. */
    if (header.id != WSF_NVM_RESERVED_FILECODE)
    {
      /* Calculate CRC of header itself. */
      headerCrc = CalcCrc32(WSF_NVM_CRC_INIT_VALUE, sizeof(header.id) + sizeof(header.len),
                            (uint8_t *)&header);

      if (headerCrc != header.headerCrc)
      {
        /* Corrupt header. */
        /* TODO: Catastrophic failure? */
        break;
      }
      else if (header.id == id)
      {
        dataCrc = CalcCrc32(WSF_NVM_CRC_INIT_VALUE, len, pData);
        if (dataCrc == header.dataCrc)
        {
          if (compCback)
          {
            compCback(TRUE);
          }
          return TRUE;
        }
        else
        {
          /* Valid header and matching ID - scratch header out. */
          header.id = WSF_NVM_RESERVED_FILECODE;
          header.headerCrc = 0;
          header.dataCrc = 0;

          am_hal_flash_program_main(
            AM_HAL_FLASH_PROGRAM_KEY, (uint32_t *)&header,
            (uint32_t *)storageAddr, sizeof(header) / WSF_FLASH_WORD_SIZE);
        }
      }
    }

    /* Move to next stored data block and read header. */
    storageAddr += WSF_NVM_WORD_ALIGN(header.len) + sizeof(header);
    memcpy(&header, (const void *)storageAddr, sizeof(header));
  } while(1);

  /* After cycling through all headers, create a new stored data header and store data */
  header.id = id;
  header.len = len;
  header.headerCrc = CalcCrc32(WSF_NVM_CRC_INIT_VALUE, sizeof(header.id) + sizeof(header.len),
                               (uint8_t *)&header);
  header.dataCrc = CalcCrc32(WSF_NVM_CRC_INIT_VALUE, len, pData);
  am_hal_flash_program_main(
    AM_HAL_FLASH_PROGRAM_KEY, (uint32_t *)&header,
    (uint32_t *)storageAddr, sizeof(header) / WSF_FLASH_WORD_SIZE);
  am_hal_flash_program_main(
    AM_HAL_FLASH_PROGRAM_KEY, (uint32_t *)pData,
    (uint32_t *)(storageAddr + sizeof(header)), len / WSF_FLASH_WORD_SIZE);

  /* Move to next empty flash. */
  storageAddr += WSF_NVM_WORD_ALIGN(header.len) + sizeof(header);
  wsfNvmCb.availAddr = storageAddr;

  if (compCback)
  {
    compCback((wsfNvmCb.availAddr - WSF_NVM_START_ADDR) <= wsfNvmCb.totalSize);
  }

  return TRUE;
}

/*************************************************************************************************/
/*!
 *  \brief  Erase data.
 *
 *  \param  id         Erase ID.
 *  \param  compCback  Write callback.
 *
 *  \return TRUE if NVM operation is successful, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t WsfNvmEraseData(uint64_t id, WsfNvmCompEvent_t compCback)
{
  WsfNvmHeader_t header;
  uint32_t headerCrc;
  uint32_t storageAddr = WSF_NVM_START_ADDR;
  bool_t erased = FALSE;

  WSF_ASSERT(!((id == WSF_NVM_RESERVED_FILECODE) || (id == WSF_NVM_UNUSED_FILECODE)));

  /* Read first header. */
  memcpy(&header, (const void *)storageAddr, sizeof(header));

  do
  {
    if (header.id == WSF_NVM_UNUSED_FILECODE)
    {
      /* Found unused entry at end of used storage. */
      break;
    }

    /* Iterate through stored data headers, looking for existing matching stored data header. */
    if (header.id != WSF_NVM_RESERVED_FILECODE)
    {
      headerCrc = CalcCrc32(WSF_NVM_CRC_INIT_VALUE, sizeof(header.id) + sizeof(header.len),
                            (uint8_t *)&header);

      if (headerCrc != header.headerCrc)
      {
        /* Corrupt header. */
        /* TODO: Catastrophic failure? */
        break;
      }
      else if (header.id == id)
      {
        header.id = WSF_NVM_RESERVED_FILECODE;
        header.headerCrc = 0;
        header.dataCrc = 0;
        am_hal_flash_program_main(
          AM_HAL_FLASH_PROGRAM_KEY, (uint32_t *)&header,
          (uint32_t *)storageAddr, sizeof(header) / WSF_FLASH_WORD_SIZE);
        erased = TRUE;
      }
    }

    /* Move to next stored data block and read header. */
    storageAddr += WSF_NVM_WORD_ALIGN(header.len) + sizeof(header);
    memcpy(&header, (const void *)storageAddr, sizeof(header));
  } while(1);

  if (compCback)
  {
    compCback(erased);
  }
  return erased;
}

/*************************************************************************************************/
/*!
 *  \brief  Erase all data located in NVM storage.
 *
 *  \param  compCback          Erase callback.
 *
 *  \note   Security Risk Warning. NVM storage could be shared by multiple Apps.
 */
/*************************************************************************************************/
void WsfNvmEraseDataAll(WsfNvmCompEvent_t compCback)
{
  for (uint32_t eraseAddr = WSF_NVM_START_ADDR; eraseAddr < wsfNvmCb.availAddr; eraseAddr += wsfNvmCb.sectorSize)
  {
    am_hal_flash_page_erase(
        AM_HAL_FLASH_PROGRAM_KEY,
        AM_HAL_FLASH_ADDR2INST(eraseAddr),
        AM_HAL_FLASH_ADDR2PAGE(eraseAddr));
  }
  wsfNvmCb.availAddr = WSF_NVM_START_ADDR;

  if (compCback)
  {
    compCback(TRUE);
  }
}
