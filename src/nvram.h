#include <timer.h>

int nvram_read_wear_leveled(int flash_base, void *buf, size_t record_size);
int nvram_read_sector(int flash_base, void *buf, int record_size);