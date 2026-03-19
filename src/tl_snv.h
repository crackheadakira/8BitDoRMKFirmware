#include <stdint.h>
#include <timer.h>

int flash_read_wear_leveled(int flash_base, void *buf, size_t record_size);
int flash_read_sector(int flash_base, void *buf, int record_size);

extern uint32_t flash_bank_address_1;
extern uint32_t flash_bank_address_2;