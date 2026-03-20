#include "kbd_usb.h"

#include "drivers/B87/lib/include/pm.h"
#include "drivers/B87/flash.h"
#include "drivers/B87/timer.h"
#include "flash/flash_mid13325e.h"
#include "flash/flash_mid1360c8.h"
#include "common/string.h"

#define FLASH_BASE_1 0x69000
#define FLASH_BASE_2 0x69800

#define WEAR_LEVEL_LIMIT_1 0x79C
#define WEAR_LEVEL_LIMIT_2 0x384

#define SECTOR_SIZE 0x1000
#define SECTOR_SIZE_LIMIT 3000

/**
 * @brief       Counts the number of bytes in a buffer that differ from a target value.
 *
 * @param       buffer  Pointer to the buffer to scan.
 * @param       target  Byte value to compare each element against.
 * @param       length  Number of bytes to scan from the start of buffer
 *
 * @return      Count of bytes that did not match target.
 *
 * @fw_addr     0x000080AC
 * @fw_ver      1.3.6r
 * @source      re
 */
static size_t memcount_ne(const uint8_t *buffer, uint8_t target, size_t length)
{
    size_t count = 0;

    for (int i = 0; i < length; i++)
    {
        if (buffer[i] != target)
        {
            count++;
        }
    }

    return count;
}

int32_t flash_read_wear_leveled(uint32_t flash_base, void *buf, size_t record_size)
{
    size_t scan_limit;

    if (flash_base == FLASH_BASE_1)
        scan_limit = WEAR_LEVEL_LIMIT_1 - record_size;
    else
        scan_limit = WEAR_LEVEL_LIMIT_2 - record_size;

    int offset;
    for (offset = 0; offset < scan_limit; offset += record_size)
    {
        flash_read_data(flash_base + offset, record_size, buf);

        if (memcount_ne(buf, 0xFF, record_size) == 0)
            break;
    }

    offset -= record_size;

    if (offset < 0)
    {
        if (flash_base == FLASH_BASE_2)
            memset(buf, 0, record_size);
    }
    else if (offset <= WEAR_LEVEL_LIMIT_1)
    {
        flash_read_data(flash_base + offset, record_size, buf);
    }
    else
    {
        flash_erase_sector(flash_base);
        sleep_us(10);

        flash_read_data(flash_base, record_size, buf);
        offset = 0;
    }

    return offset;
}

int32_t flash_read_sector(uint32_t flash_base, void *buf, size_t record_size)
{
    if (record_size >= SECTOR_SIZE)
        return 0;

    int offset;
    for (offset = 0; offset <= (SECTOR_SIZE - record_size); offset += record_size)
    {
        flash_read_data(flash_base + offset, record_size, buf);

        if (memcount_ne(buf, 0xFF, record_size) == 0)
            break;
    }

    // Step back one record to point to the last valid data entry
    offset -= record_size;

    if (offset < 0)
    {
        flash_read_data(flash_base, record_size, buf);
    }
    else
    {
        flash_read_data(flash_base + offset, record_size, buf);

        if (offset > SECTOR_SIZE_LIMIT)
        {
            flash_erase_sector(flash_base);
            sleep_us(10);

            flash_read_data(flash_base, record_size, buf);
            offset = 0;
        }
    }

    return offset;
}

void flash_lock_by_mid(void)
{
    flash_mid_e flash_mid = flash_read_mid();

    if (flash_mid == MID13325E)
    {
        flash_lock_mid13325e(0x18);
    }
    else if (flash_mid == MID1360C8)
    {
        flash_lock_mid1360c8(0x18);
    }
}

void flash_unlock_by_mid(void)
{
    flash_mid_e flash_mid = flash_read_mid();

    if (flash_mid == MID13325E)
    {
        flash_unlock_mid13325e();
    }
    else if (flash_mid == MID1360C8)
    {
        flash_unlock_mid1360c8();
    }
}