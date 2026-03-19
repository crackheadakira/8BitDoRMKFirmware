#include <stdint.h>

uint32_t dfu_hardware_init(void);
void rf_reboot(uint8_t channel);

extern uint8_t dfu_active;
extern uint8_t rf_channel;