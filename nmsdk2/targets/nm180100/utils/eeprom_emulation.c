/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2021, Northern Mechatronics, Inc.
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <am_hal_flash.h>

#include "eeprom_emulation.h"

#define EEPROM_UNALLOCATED    0x00
#define EEPROM_ALLOCATED      0x01

#define SIZE_OF_DATA 2                                            /* 2 bytes */
#define SIZE_OF_VIRTUAL_ADDRESS 2                                 /* 2 bytes */
#define SIZE_OF_VARIABLE (SIZE_OF_DATA + SIZE_OF_VIRTUAL_ADDRESS) /* 4 bytes */

#define MAX_ACTIVE_VARIABLES (AM_HAL_FLASH_PAGE_SIZE / SIZE_OF_VARIABLE) - 1

typedef enum {
    EEPROM_PAGE_STATUS_ERASED = 0xFF,
    EEPROM_PAGE_STATUS_RECEIVING = 0xAA,
    EEPROM_PAGE_STATUS_ACTIVE = 0x00,
} eeprom_page_status_e;

/* Since the data to be written to flash must be read from ram, the data used to
 * set the page status is explicitly written to the ram beforehand. */
static uint32_t EEPROM_PAGE_STATUS_ACTIVE_VALUE =
    ((uint32_t)EEPROM_PAGE_STATUS_ACTIVE << 24) | 0x00FFFFFF;
static uint32_t EEPROM_PAGE_STATUS_RECEIVING_VALUE =
    ((uint32_t)EEPROM_PAGE_STATUS_RECEIVING << 24) | 0x00FFFFFF;

static inline eeprom_page_status_e eeprom_page_get_status(eeprom_page_t *psPage)
{
    return (eeprom_page_status_e)((*(psPage->pui32StartAddress) >> 24) & 0xFF);
}

static inline int eeprom_page_set_active(eeprom_page_t *psPage)
{
    return am_hal_flash_program_main(
        AM_HAL_FLASH_PROGRAM_KEY, &EEPROM_PAGE_STATUS_ACTIVE_VALUE,
        psPage->pui32StartAddress, SIZE_OF_VARIABLE >> 2);
}

static inline int eeprom_page_set_receiving(eeprom_page_t *psPage)
{
    return am_hal_flash_program_main(
        AM_HAL_FLASH_PROGRAM_KEY, &EEPROM_PAGE_STATUS_RECEIVING_VALUE,
        psPage->pui32StartAddress, SIZE_OF_VARIABLE >> 2);
}

static bool eeprom_page_validate_empty(eeprom_page_t *psPage)
{
    uint32_t *address = psPage->pui32StartAddress;

    while (address <= psPage->pui32EndAddress) {
        uint32_t value = *address;
        if (value != 0xFFFFFFFF) {
            return false;
        }
        address++;
    }

    return true;
}

static bool eeprom_page_write(eeprom_page_t *psPage, uint16_t ui16VirtualAddress,
                              uint16_t ui16Data)
{
    /* Start at the second word. The fist one is reserved for status and erase count. */
    uint32_t *address = psPage->pui32StartAddress + 1;
    uint32_t virtualAddressAndData;

    /* Iterate through the page from the beginning, and stop at the fist empty word. */
    while (address <= psPage->pui32EndAddress)
    {
        /* Empty word found. */
        if (*address == 0xFFFFFFFF)
        {
            virtualAddressAndData =
                ((uint32_t)(ui16VirtualAddress << 16) & 0xFFFF0000) |
                (uint32_t)(ui16Data);

            if (am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY,
                                          &virtualAddressAndData, address,
                                          SIZE_OF_VARIABLE >> 2) != 0)
            {
                return false;
            }
            return true;
        }
        else
        {
            address++;
        }
    }

    return false;
}

static int eeprom_page_transfer(eeprom_handle_t *psHandle, uint16_t virtual_address, uint16_t data)
{
    int status;
    uint32_t *pui32ActiveAddress;
    uint32_t *pui32ReceivingAddress;
    uint32_t *pui32EndAddress;
    uint32_t ui32EraseCount;
    bool bNewData = false;

    /* If there is no receiving page predefined, set it to cycle through all ui8Allocated psPages. */
    if (psHandle->i16ReceivingPage == -1)
    {
        psHandle->i16ReceivingPage = psHandle->i16ActivePage + 1;

        if (psHandle->i16ReceivingPage >= psHandle->i16AllocatedPages)
        {
            psHandle->i16ReceivingPage = 0;
        }

        /* Check if the new receiving page really is erased. */
        if (!eeprom_page_validate_empty(&(psHandle->psPages[psHandle->i16ReceivingPage])))
        {
            /* If this page is not truly erased, it means that it has been written to
             * from outside this API, this could be an address conflict. */
            am_hal_flash_page_erase(
                AM_HAL_FLASH_PROGRAM_KEY,
                AM_HAL_FLASH_ADDR2INST(
                    (uint32_t)(psHandle->psPages[psHandle->i16ReceivingPage].pui32StartAddress)),
                AM_HAL_FLASH_ADDR2PAGE(
                    (uint32_t)(psHandle->psPages[psHandle->i16ReceivingPage].pui32StartAddress)));
        }
    }

    /* Set the status of the receiving page */
    eeprom_page_set_receiving(&(psHandle->psPages[psHandle->i16ReceivingPage]));

    /* If an address was specified, write it to the receiving page */
    if (virtual_address != 0)
    {
        eeprom_page_write(&(psHandle->psPages[psHandle->i16ReceivingPage]), virtual_address, data);
    }

    /* Start at the last word. */
    pui32ActiveAddress = psHandle->psPages[psHandle->i16ActivePage].pui32EndAddress;

    /* Iterate through all words in the active page. Each time a new virtual
     * address is found, write it and it's data to the receiving page */
    while (pui32ActiveAddress > psHandle->psPages[psHandle->i16ActivePage].pui32StartAddress)
    {
        // 0x0000 and 0xFFFF are not valid virtual addresses.
        if ((uint16_t)(*pui32ActiveAddress >> 16) == 0x0000 ||
            (uint16_t)(*pui32ActiveAddress >> 16) == 0xFFFF)
        {
            bNewData = false;
        }
        /*
        // Omit when transfer is initiated from inside the eeprom_init() function.
        else if (address != 0 && (uint16_t)(*pui32ActiveAddress >> 16) > numberOfVariablesDeclared)
        {
        // A virtual address outside the virtual address space, defined by the
        // number of variables declared, are considered garbage.
        newVariable = false;
        }
        */
        else
        {
            pui32ReceivingAddress =
                psHandle->psPages[psHandle->i16ReceivingPage].pui32StartAddress + 1;

            /* Start at the beginning of the receiving page. Check if the variable is
             * already transfered. */
            pui32EndAddress = psHandle->psPages[psHandle->i16ReceivingPage].pui32EndAddress;
            while (pui32ReceivingAddress <= pui32EndAddress)
            {
                /* Variable found, and is therefore already transferred. */
                if ((uint16_t)(*pui32ActiveAddress >> 16) ==
                    (uint16_t)(*pui32ReceivingAddress >> 16))
                {
                    bNewData = false;
                    break;
                }
                /* Empty word found. All transferred variables are checked.  */
                else if (*pui32ReceivingAddress == 0xFFFFFFFF)
                {
                    bNewData = true;
                    break;
                }
                pui32ReceivingAddress++;
            }
        }

        if (bNewData)
        {
            /* Write the new variable to the receiving page. */
            eeprom_page_write(&(psHandle->psPages[psHandle->i16ReceivingPage]),
                              (uint16_t)(*pui32ActiveAddress >> 16),
                              (uint16_t)(*pui32ActiveAddress));
        }
        pui32ActiveAddress--;
    }

    /* Update erase count */
    ui32EraseCount = eeprom_erase_counter(psHandle);

    /* If a new page cycle is started, increment the erase count. */
    if (psHandle->i16ReceivingPage == 0)
        ui32EraseCount++;

    /* Set the first byte, in this way the page status is not altered when the erase count is written. */
    ui32EraseCount = ui32EraseCount | 0xFF000000;

    /* Write the erase count obtained to the active page head. */
    status = am_hal_flash_program_main(
        AM_HAL_FLASH_PROGRAM_KEY, &ui32EraseCount,
        psHandle->psPages[psHandle->i16ReceivingPage].pui32StartAddress, SIZE_OF_VARIABLE >> 2);
    if (status != 0)
    {
        return status;
    }

    /* Erase the old active page. */
    status = am_hal_flash_page_erase(
        AM_HAL_FLASH_PROGRAM_KEY,
        AM_HAL_FLASH_ADDR2INST(
            (uint32_t)(psHandle->psPages[psHandle->i16ActivePage].pui32StartAddress)),
        AM_HAL_FLASH_ADDR2PAGE(
            (uint32_t)(psHandle->psPages[psHandle->i16ActivePage].pui32StartAddress)));
    if (status != 0)
    {
        return status;
    }

    /* Set the receiving page to be the new active page. */
    status = eeprom_page_set_active(&(psHandle->psPages[psHandle->i16ReceivingPage]));
    if (status != 0)
    {
        return status;
    }

    psHandle->i16ActivePage = psHandle->i16ReceivingPage;
    psHandle->i16ReceivingPage = -1;

    return 0;
}

uint32_t eeprom_init(uint32_t ui32StartAddress, uint32_t ui32NumberOfPages, eeprom_handle_t *psHandle)
{
    if (psHandle == NULL)
    {
        return false;
    }

    if (ui32NumberOfPages < 2)
    {
        return false;
    }

    if (psHandle->psPages == NULL)
    {
        return false;
    }

    psHandle->ui8Allocated = EEPROM_ALLOCATED;
    psHandle->i16ActivePage = -1;
    psHandle->i16ReceivingPage = -1;
    psHandle->i16AllocatedPages = ui32NumberOfPages;

    /* Initialize the address of each page */
    uint32_t i;
    for (i = 0; i < ui32NumberOfPages; i++)
    {
        uint32_t ui32PageStart = ui32StartAddress + i * AM_HAL_FLASH_PAGE_SIZE;
        uint32_t ui32PageEnd = ui32PageStart + (AM_HAL_FLASH_PAGE_SIZE) - 4;
        psHandle->psPages[i].pui32StartAddress = (uint32_t *)(ui32PageStart);
        psHandle->psPages[i].pui32EndAddress = (uint32_t *)(ui32PageEnd);
    }

    /* Check status of each page */
    for (i = 0; i < ui32NumberOfPages; i++)
    {
        switch (eeprom_page_get_status(&(psHandle->psPages[i])))
        {
        case EEPROM_PAGE_STATUS_ACTIVE:
            if (psHandle->i16ActivePage == -1) {
                psHandle->i16ActivePage = i;
            } else {
                // More than one active page found. This is an invalid system state.
                return false;
            }
            break;
        case EEPROM_PAGE_STATUS_RECEIVING:
            if (psHandle->i16ReceivingPage == -1) {
                psHandle->i16ReceivingPage = i;
            } else {
                // More than one receiving page found. This is an invalid system state.
                return false;
            }
            break;
        case EEPROM_PAGE_STATUS_ERASED:
            // Validate if the page is really erased, and erase it if not.
            if (!eeprom_page_validate_empty(&(psHandle->psPages[i]))) {
                am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY,
                                        AM_HAL_FLASH_ADDR2INST((uint32_t)(
                                            psHandle->psPages[i].pui32StartAddress)),
                                        AM_HAL_FLASH_ADDR2PAGE((uint32_t)(
                                            psHandle->psPages[i].pui32StartAddress)));
            }
            break;
        default:
            // Undefined page status, erase page.
            am_hal_flash_page_erase(
                AM_HAL_FLASH_PROGRAM_KEY,
                AM_HAL_FLASH_ADDR2INST((uint32_t)(psHandle->psPages[i].pui32StartAddress)),
                AM_HAL_FLASH_ADDR2PAGE((uint32_t)(psHandle->psPages[i].pui32StartAddress)));
            break;
        }
    }

    if ((psHandle->i16ReceivingPage == -1) && (psHandle->i16ActivePage == -1)) {
        return false;
    }

    if (psHandle->i16ReceivingPage == -1) {
        return true;
    } else if (psHandle->i16ActivePage == -1) {
        psHandle->i16ActivePage = psHandle->i16ReceivingPage;
        psHandle->i16ReceivingPage = -1;
        eeprom_page_set_active(&(psHandle->psPages[psHandle->i16ActivePage]));
    } else {
        eeprom_page_transfer(psHandle, 0, 0);
    }

    return true;
}

uint32_t eeprom_format(eeprom_handle_t *psHandle)
{
    uint32_t ui32EraseCount = 0xFF000001;
    int i;
    int status;

    for (i = psHandle->i16AllocatedPages - 1; i >= 0; i--)
    {
        if (!eeprom_page_validate_empty(&(psHandle->psPages[i])))
        {
            status = am_hal_flash_page_erase(
                AM_HAL_FLASH_PROGRAM_KEY,
                AM_HAL_FLASH_ADDR2INST((uint32_t)(psHandle->psPages[i].pui32StartAddress)),
                AM_HAL_FLASH_ADDR2PAGE((uint32_t)(psHandle->psPages[i].pui32StartAddress)));
            if (status != 0)
            {
                return false;
            }
        }
    }

    psHandle->i16ActivePage = 0;
    psHandle->i16ReceivingPage = -1;

    status = am_hal_flash_program_main(
        AM_HAL_FLASH_PROGRAM_KEY, &ui32EraseCount,
        psHandle->psPages[psHandle->i16ActivePage].pui32StartAddress, 1);

    if (status != 0)
    {
        return false;
    }

    status = eeprom_page_set_active(&(psHandle->psPages[psHandle->i16ActivePage]));
    if (status != 0)
    {
        return false;
    }

    return true;
}

uint32_t eeprom_read(eeprom_handle_t *psHandle, uint16_t ui16VirtualAddress, uint16_t *pui16Data)
{
    uint32_t *pui32Address;

    if (!psHandle->ui8Allocated) {
        return false;
    }

    pui32Address = (psHandle->psPages[psHandle->i16ActivePage].pui32EndAddress);

    // 0x0000 and 0xFFFF are illegal addresses.
    if (ui16VirtualAddress != 0x0000 && ui16VirtualAddress != 0xFFFF) {
        while (pui32Address > psHandle->psPages[psHandle->i16ActivePage].pui32StartAddress) {
            if ((uint16_t)(*pui32Address >> 16) == ui16VirtualAddress) {
                *pui16Data = (uint16_t)(*pui32Address);
                return true;
            }
            pui32Address--;
        }
    }
    // Variable not found, return null value.
    *pui16Data = 0x0000;

    return false;
}

uint32_t eeprom_read_array(eeprom_handle_t *psHandle, uint16_t ui16VirtualAddress, uint8_t *pui8Data, uint8_t *pui8Length)
{
    if (!psHandle->ui8Allocated) {
        return false;
    }

    uint16_t value;
    if (!eeprom_read(psHandle, ui16VirtualAddress, &value))
    {
        return false;
    }

    *pui8Length = (value >> 8) & 0xFF;
    pui8Data[0] = value & 0xFF;
    for (int i = 1; i < *pui8Length; i++)
    {
        if (!eeprom_read(psHandle, ui16VirtualAddress + i, &value))
        {
            *pui8Length = i;
            return false;
        }
        pui8Data[i] = value & 0xFF;
    }

    return true;
}

uint32_t eeprom_read_array_len(eeprom_handle_t *psHandle, uint16_t ui16VirtualAddress, uint8_t *pui8Data, uint16_t ui16Size)
{
    uint16_t value;

    if (!psHandle->ui8Allocated) {
        return false;
    }

    for (int i = 0; i < ui16Size; i++)
    {
        if (!eeprom_read(psHandle, ui16VirtualAddress + i, &value))
        {
            return false;
        }
        pui8Data[i] = value & 0xFF;
    }

    return true;
}

uint32_t eeprom_write(eeprom_handle_t *psHandle, uint16_t ui16VirtualAddress, uint16_t ui16Data)
{
    if (!psHandle->ui8Allocated) {
        return false;
    }

    uint16_t stored_value;

    if (eeprom_read(psHandle, ui16VirtualAddress, &stored_value)) {
        if (stored_value == ui16Data) {
            return true;
        }
    }

    if (!eeprom_page_write(&(psHandle->psPages[psHandle->i16ActivePage]), ui16VirtualAddress, ui16Data)) {
        eeprom_page_transfer(psHandle, ui16VirtualAddress, ui16Data);
    }

    return true;
}

uint32_t eeprom_write_array(eeprom_handle_t *psHandle, uint16_t ui16VirtualAddress, uint8_t *pui8Data, uint8_t ui8Length)
{
    if (!psHandle->ui8Allocated) {
        return false;
    }

    uint16_t stored_value;

    if (eeprom_read(psHandle, ui16VirtualAddress, &stored_value)) {
        uint8_t stored_len = (stored_value >> 8) & 0xFF;
        if (stored_len != ui8Length) {
            for (int i = 0; i < stored_len; i++)
            {
                eeprom_delete(psHandle, ui16VirtualAddress + i);
            }
        }
    }

    uint16_t value = (ui8Length << 8) | pui8Data[0];
    if (!eeprom_page_write(
            &(psHandle->psPages[psHandle->i16ActivePage]),
            ui16VirtualAddress, value)) {
        eeprom_page_transfer(psHandle, ui16VirtualAddress, value);
    }

    for (int i = 1; i < ui8Length; i++)
    {
        if (!eeprom_page_write(
                &(psHandle->psPages[psHandle->i16ActivePage]), ui16VirtualAddress + i, pui8Data[i])) {
            eeprom_page_transfer(psHandle, ui16VirtualAddress + i, pui8Data[i]);
        }
    }
    
    return true;
}

uint32_t eeprom_write_array_len(eeprom_handle_t *psHandle, uint16_t ui16VirtualAddress, uint8_t *pui8Data, uint16_t ui16Size)
{
    if (!psHandle->ui8Allocated) {
        return false;
    }

    for (int i = 0; i < ui16Size; i++)
    {
        eeprom_write(psHandle, ui16VirtualAddress + i, pui8Data[i]);
    }

    return true;
}

uint32_t eeprom_delete(eeprom_handle_t *psHandle, uint16_t ui16VirtualAddress)
{
    if (!psHandle->ui8Allocated) {
        return false;
    }

    bool bDeleted = false;

    uint32_t data = 0x0000FFFF;
    uint32_t *address = psHandle->psPages[psHandle->i16ActivePage].pui32EndAddress;

    while (address > psHandle->psPages[psHandle->i16ActivePage].pui32StartAddress)
    {
        if ((uint16_t)(*address >> 16) == ui16VirtualAddress)
        {
            bDeleted = true;
            am_hal_flash_program_main(
                  AM_HAL_FLASH_PROGRAM_KEY,
                  &data, address,
                  SIZE_OF_VARIABLE >> 2);
        }
        address--;
    }

    return bDeleted;
}

uint32_t eeprom_delete_array(eeprom_handle_t *psHandle, uint16_t ui16VirtualAddress)
{
    if (!psHandle->ui8Allocated) {
        return false;
    }

    bool bDeleted = false;
    uint16_t stored_value;

    if (eeprom_read(psHandle, ui16VirtualAddress, &stored_value))
    {
        uint8_t stored_len = (stored_value >> 8) & 0xFF;
        for (int i = 0; i < stored_len; i++)
        {
            bDeleted |= eeprom_delete(psHandle, ui16VirtualAddress + i);
        }
    }

    return bDeleted;
}

uint32_t eeprom_erase_counter(eeprom_handle_t *psHandle)
{
    if (psHandle->i16ActivePage == -1)
    {
        return 0xFFFFFF;
    }

    uint32_t eraseCount;

    /* The number of erase cycles is the 24 LSB of the first word of the active page. */
    eraseCount = (*(psHandle->psPages[psHandle->i16ActivePage].pui32StartAddress) & 0x00FFFFFF);

    /* if the page has never been erased, return 0. */
    if (eraseCount == 0xFFFFFF) {
        return 0;
    }

    return eraseCount;
}
