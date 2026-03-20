/**
 * @file    tl_snv.h
 *
 * @brief   Header file for functions related to flash.
 *
 * @author  crackheadakira
 * @date    March 20, 2026
 *
 * @fw_ver  1.3.6r
 */

#pragma once
#include "common/types.h"
#include <stdint.h>

extern uint32_t flash_bank_address_1;
extern uint32_t flash_bank_address_2;
extern uint32_t flash_timeout_1;

/**
 * @brief   Reads the most recent valid record from a wear-leveled flash region.
 *
 * @details Wear leveling spreads writes across a flash region to avoid
 *          repeatedly erasing the same sector. Records are written
 *          sequentially — a blank record (all 0xFF) marks the end of
 *          written data. This function scans forward to find the last
 *          written record and returns it.
 *
 *          The scan limit differs between the two known flash bases:
 *          - FLASH_BASE_1: scans up to WEAR_LEVEL_LIMIT_1
 *          - FLASH_BASE_2: scans up to WEAR_LEVEL_LIMIT_2
 *
 *          If the region is full (no blank record found), the sector is
 *          erased and reading restarts from the beginning.
 *
 *          If no valid record exists and flash_base == FLASH_BASE_2,
 *          the buffer is zeroed rather than left with 0xFF values.
 *
 * @note    May erase the flash sector if the region is full.
 *          Calls sleep_us(10) after erase before reading.
 *          Only FLASH_BASE_1 and FLASH_BASE_2 are valid inputs —
 *          behavior is undefined for other base addresses.
 *
 * @param   flash_base    Base address of the wear-leveled flash region.
 *                           Must be FLASH_BASE_1 or FLASH_BASE_2.
 * @param   buf           Output buffer to receive the record data.
 * @param   record_size   Size of each record in bytes.
 *
 * @return  Byte offset of the record that was read, relative to flash_base.
 *          Returns a negative value if no valid record was found.
 *
 * @fw_addr 0x00009710
 * @fw_ver  1.3.6r
 * @source  re
 */
int32_t flash_read_wear_leveled(uint32_t flash_base, void *buf, size_t record_size);

/**
 * @brief   Reads the most recent valid record from a single flash sector.
 *
 * @details Similar to flash_read_wear_leveled but operates on a single
 *          fixed-size sector (SECTOR_SIZE bytes). Records are written
 *          sequentially within the sector — a blank record (all 0xFF)
 *          marks the end of written data.
 *
 *          Scans forward through the sector in record_size steps until
 *          a blank record is found, then steps back one record to read
 *          the last valid entry.
 *
 *          If the sector is full (offset exceeds SECTOR_SIZE_LIMIT),
 *          the sector is erased and reading restarts from offset 0.
 *
 *          If no valid record exists (offset went negative), reads
 *          from the base address directly — the sector may be blank.
 *
 * @note    record_size must be smaller than SECTOR_SIZE — returns 0
 *          immediately if this condition is not met.
 *          May erase the flash sector if it is full.
 *          Calls sleep_us(10) after erase before reading.
 *
 * @param   flash_base    Base address of the flash sector to read from.
 * @param   buf           Output buffer to receive the record data.
 * @param   record_size   Size of each record in bytes. Must be < SECTOR_SIZE.
 *
 * @return  Byte offset of the record that was read, relative to flash_base.
 *          Returns 0 if record_size >= SECTOR_SIZE, after sector erase, or
 *          when offset went negative.
 *
 * @fw_addr 0x00009668
 * @fw_ver  1.3.6r
 * @source  re
 */
int32_t flash_read_sector(uint32_t flash_base, void *buf, size_t record_size);

/**
 * @brief   Locks flash and routes it to it's respective flash_lock
 *          function, dependent on MID.
 *
 * @details Reads the flash manufacturer ID and dispatches to the
 *          appropriate lock function with protection level 0x18.
 *          If the MID is not recognized, no action is taken and
 *          flash remains in its current state.
 *
 *          Supported MIDs:
 *            MID13325E — XTX flash
 *            MID1360C8 — GigaDevice flash
 *
 * @note    MID136085 and MID1360EB are not supported — these variants
 *          only exist in the B85 SDK which targets a different MCU.
 *          Silently does nothing for unrecognized MIDs.
 *
 * @fw_addr 0x00000338
 * @fw_ver  1.3.6r
 * @source  re
 */
void flash_lock_by_mid(void);

/**
 * @brief   Unlocks flash and routes it to it's respective flash_unlock
 *          function, dependent on MID.
 *
 * @details Reads the flash manufacturer ID and dispatches to the
 *          appropriate unlock function.
 *          If the MID is not recognized, no action is taken and
 *          flash remains in its current state.
 *
 *          Supported MIDs:
 *            MID13325E — XTX flash
 *            MID1360C8 — GigaDevice flash
 *
 * @note    MID136085 and MID1360EB are not supported — these variants
 *          only exist in the B85 SDK which targets a different MCU.
 *          Silently does nothing for unrecognized MIDs.
 *
 * @fw_addr 0x00000388
 * @fw_ver  1.3.6r
 * @source  re
 */
void flash_unlock_by_mid(void);