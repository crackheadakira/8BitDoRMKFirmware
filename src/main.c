#include "stack/ble/ble.h"
#include "drivers.h"
#include "app_battery.h"
#include "app_config.h"
#include "ble.h"
#include "connection.h"
#include "nvram.h"

typedef struct
{
    void *ptr_peripheral_ctrl;
    void *ptr_io_map;
    uint32_t nvram_config_data;
    uint32_t last_poll_tick;
    uint32_t current_key_report;
} device_state_t;

device_state_t device_state;

uint8_t saved_conn_mode;
uint8_t saved_profile_id;
uint8_t saved_sleep_state;

void irq_handler(void)
{
}

uint32_t ble_stack_param_init(void)
{
    return 0;
}

int main(void)
{
    cpu_wakeup_init(DCDC_MODE, INTERNAL_CAP_XTAL24M);

    int has_mcu_awoken = pm_is_MCU_deepRetentionWakeup();

    gpio_init(has_mcu_awoken);

    if (has_mcu_awoken == 0)
    {
        conn_mode_init();
    }

    clock_init(SYS_CLK_48M_Crystal);

    if (has_mcu_awoken == 0)
    {
        // TODO: Create a flash struct that then will be used for calculation
        // as 847b70 will be set to 0x20000 - *847b84.
        // *847b84 is being set by a function inside of cpu_wakeup_init.
        uint32_t sram_value_unknown = 0x20000;
        initialize_adc(sram_value_unknown + (GPIO_PA3 | GPIO_PA4), B3P);
        initialize_adc(sram_value_unknown - GPIO_PA2, B3P);
        initialize_adc(sram_value_unknown + GPIO_PA1, B3P);

        flash_mid_e flash_mid = flash_read_mid();

        if (flash_mid == MID13325E)
        {
            flash_write_status_mid13325e(0x18, 0x1C);
        }
        else if (flash_mid == MID1360C8)
        {
            flash_write_status_mid1360c8(0x18, 0x1C);
        }

        // The value of (mid&0x00ff0000)>>16 reflects flash capacity.
        uint32_t flash_capacity = (flash_mid & 0x00FF0000) >> 16;

        if (flash_capacity == FLASH_SIZE_512K)
        {
            // set flash_bank_addr to CFG_ADR_MAC_512K_FLASH
            // set (flash_bank_addr + 4) to CFG_ADR_CALIBRATION_512K_FLASH
        }
        else if (flash_capacity == FLASH_SIZE_1M)
        {
            // set flash_bank_addr to CFG_ADR_MAC_1M_FLASH
            // set (flash_bank_addr + 4) to CFG_ADR_CALIBRATION_1M_FLASH
        }
        else
        {
            while
                true {};
        }

        analog_read(DEEP_ANA_REG0);
        // *connection_mode_ptr = above ^;

        analog_read(DEEP_ANA_REG1);
        // *PTR_DAT_00000758 = above ^;

        analog_read(DEEP_ANA_REG2);
        // *PTR_DAT_0000075c = above ^; seems to not be read in code

        ble_identity_init();

        uint8_t g_KeymapProfile[0x20];
        int nv_config_offset = nvram_read_sector(0x79000, g_KeymapProfile, 0x20);

        uint8_t nv_bond_buf[0x50];
        int nv_bond_offset = nvram_read_wear_leveled(0x69000, nv_bond_buf, 0x50);

        uint8_t nv_something_buf[0x3E];
        int nv_something_offset = nvram_read_wear_leveled(0x69800, nv_something_buf, 0x3E);

        uint8_t nv_small_buf[0x12];
        int nv_small_offset = nvram_read_wear_leveled(0x69C00, nv_small_buf, 0x12);

        uint8_t nv_word_buf[4];
        int nv_word_offset = nvram_read_sector(0x73000, nv_word_buf, 0x04);
    }

    while (1)
    {
    }

    return 0;
}