#include "common/string.h"
#include "drivers/B87/flash.h"
#include "stdint.h"
#include "timer.h"

#define FLASH_BASE_1 0x69000
#define FLASH_BASE_2 0x69800

#define WEAR_LEVEL_LIMIT_1 0x79C
#define WEAR_LEVEL_LIMIT_2 0x384

#define SECTOR_SIZE 0x1000
#define SECTOR_SIZE_LIMIT 3000

size_t memcount_ne(const unsigned char *buffer, unsigned char target, size_t length)
{
    short count = 0;

    for (int i = 0; i < length; i++)
    {
        if (buffer[i] != target)
        {
            count++;
        }
    }

    return count;
}

int flash_read_wear_leveled(int flash_base, void *buf, size_t record_size)
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
        {
            break;
        }
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

int flash_read_sector(int flash_base, void *buf, int record_size)
{
    if (record_size >= SECTOR_SIZE)
        return 0;

    int offset;
    for (offset = 0; offset <= (SECTOR_SIZE - record_size); offset += record_size)
    {
        flash_read_data(flash_base + offset, record_size, buf);

        // If the record is entirely 0xFF (erased flash), we found the end of the data
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