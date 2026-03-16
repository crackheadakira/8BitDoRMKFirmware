#include "stack/ble/ble.h"
#include "drivers.h"
#include "app_battery.h"
#include "app_config.h"
#include "ble.h"
#include "connection.h"

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

void irq_handler(void) {}

void ble_stack_param_init(void) {}

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
        initialize_adc(sram_value_unknown - 4, B3P);
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

        /*
            uVar3 = analog_read(DEEP_ANA_REG0);
            unknown_ptr = PTR_DAT_00000728;
            *PTR_DAT_00000728 = uVar3;
            uVar3 = analog_read(DEEP_ANA_REG1);
            *PTR_DAT_00000758 = uVar3;
            uVar3 = analog_read(DEEP_ANA_REG2);
            *PTR_DAT_0000075c = uVar3;
        */

        analog_read(DEEP_ANA_REG0);
        analog_read(DEEP_ANA_REG1);
        analog_read(DEEP_ANA_REG2);

        ble_identity_init();
    }

    while (1)
    {
    }

    return 0;
}