#include "drivers/B87/driver.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "stdint.h"

typedef struct
{
    void *ptr_peripheral_ctrl;
    void *ptr_io_map;
    uint32_t nvram_config_data;
    uint32_t last_poll_tick;
    uint32_t current_key_report;
} device_state_t;

typedef struct
{
    uint8_t ble_identity[0x20];
    uint8_t keyboard_layout[0x50];
    uint8_t lighting_config[0x3E];
    uint8_t pairing_state[0x12];
    uint32_t active_flash_bank;
} nvram_data_t;

device_state_t device_state;
nvram_data_t nvram_state;

uint8_t saved_conn_mode;
uint8_t saved_profile_id;
uint8_t saved_sleep_state;

bool is_cold_boot; // Maps to Ghidra's *reg_system_tick_ptr == 0

void irq_handler(void)
{
}

void ble_stack_param_init(void)
{
}

int main(void)
{
    cpu_wakeup_init(LDO_MODE, INTERNAL_CAP_XTAL24M);
    clock_init(SYS_CLK_16M_Crystal);

    while (1)
    {
    }

    return 0;
}