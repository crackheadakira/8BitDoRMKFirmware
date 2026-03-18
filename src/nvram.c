#include "common/string.h"
#include "drivers/B87/flash.h"
#include "stdint.h"
#include "timer.h"

#define NVRAM_BASE 0x69000
#define WEAR_LEVEL_LIMIT 0x79C
#define SECTOR_SIZE_LIMIT 0xBB8

short memcount_ne(const char *buffer, char target, int length)
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

static int nvram_scan_last_record(int flash_base, void *buf, size_t record_size,
                                  size_t scan_limit)
{
    int i;

    for (i = 0; i < scan_limit; i += record_size)
    {
        flash_read_page(flash_base + i, record_size, buf);

        if (memcount_ne(buf, 0xFF, record_size) == 0)
            break;
    }

    return i - record_size;
}

int nvram_read_wear_leveled(int flash_base, void *buf, size_t record_size)
{
    size_t scan_limit;

    if (flash_base == NVRAM_BASE)
        scan_limit = WEAR_LEVEL_LIMIT - record_size;
    else
        scan_limit = 0x384 - record_size;

    int offset = nvram_scan_last_record(flash_base, buf, record_size, scan_limit);

    if (offset < 0)
    {
        if (flash_base == 0x69800)
            memset(buf, 0, record_size);

        return offset;
    }

    if (offset > WEAR_LEVEL_LIMIT)
    {
        flash_erase_sector(flash_base);
        WaitUs(10);
        offset = 0;
    }

    flash_read_page(flash_base + offset, record_size, buf);
    return offset;
}

int nvram_read_sector(int flash_base, void *buf, int record_size)
{
    if (record_size >= 0x1000)
        return 0;

    int offset = nvram_read_wear_leveled(flash_base, buf, record_size);

    if (offset < 0)
    {
        return nvram_read_wear_leveled(flash_base, buf, record_size);
    }

    if (offset > SECTOR_SIZE_LIMIT)
    {
        flash_erase_sector(flash_base);
        WaitUs(10);
        return nvram_read_wear_leveled(flash_base, buf, record_size);
    }

    return offset;
}