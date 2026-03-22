#include "stack/ble/ble.h"

#include "drivers.h"

#include "app_battery.h"
#include "app_config.h"
#include "ble.h"
#include "connection.h"
#include "kbd_usb.h"
#include "tl_snv.h"

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

uint32_t flash_bank_address_1;
uint32_t flash_bank_address_2;

void irq_handler(void)
{
    if (reg_irq_src & FLD_IRQ_TMR0_EN)
    {
        reg_irq_src = FLD_IRQ_TMR0_EN;
    }
    if (reg_irq_src & FLD_IRQ_TMR1_EN)
    {
        reg_irq_src = FLD_IRQ_TMR1_EN;
    }
}

uint32_t ble_stack_param_init(void)
{
    return 0;
}

void flash_set_bank_addrs(flash_mid_e flash_mid)
{
    uint32_t flash_capacity = (flash_mid & 0x00FF0000) >> 16;

    if (flash_capacity == FLASH_SIZE_512K)
    {
        flash_bank_address_1 = 0x76000;
        flash_bank_address_2 = 0x77000;
    }
    else if (flash_capacity == FLASH_SIZE_1M)
    {
        flash_bank_address_1 = 0xFF000;
        flash_bank_address_2 = 0xFE000;
    }
    else
    {
        while (1)
        {
        } // unsupported flash size
    }
}

int main(void)
{
    cpu_wakeup_init(DCDC_MODE, INTERNAL_CAP_XTAL24M);

    int has_mcu_awoken = pm_is_MCU_deepRetentionWakeup();

    gpio_init(has_mcu_awoken);

    /*
    if (has_mcu_awoken == 0)
    {
        conn_mode_init();
    }
    */

    clock_init(SYS_CLK_48M_Crystal);

    /*
    if (has_mcu_awoken == 0)
    {
        uint32_t ota_flash = ota_program_bootAddr - ota_program_offset;
        uint32_t firmware_buf;
        flash_read_page(ota_flash + 24, 4, (uint8_t *)&firmware_buf);
        flash_read_page(ota_flash + (has_mcu_awoken - 4), 4, device_state.ptr_peripheral_ctrl);
        flash_read_page(ota_flash + 2, 4, device_state.ptr_io_map);

        flash_lock_by_mid();

        flash_mid_e flash_mid = flash_read_mid();

        // The value of (mid & 0x00FF0000) >> 16 reflects flash capacity.
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
        int nv_config_offset = flash_read_sector(0x79000, g_KeymapProfile, 0x20);

        uint8_t nv_bond_buf[0x50];
        int nv_bond_offset = flash_read_wear_leveled(0x69000, nv_bond_buf, 0x50);

        uint8_t nv_something_buf[0x3E];
        int nv_something_offset = flash_read_wear_leveled(0x69800, nv_something_buf, 0x3E);

        uint8_t nv_small_buf[0x12];
        int nv_small_offset = flash_read_wear_leveled(0x69C00, nv_small_buf, 0x12);

        uint8_t nv_word_buf[4];
        int nv_word_offset = flash_read_sector(0x73000, nv_word_buf, 0x04);
    }
    */

    flash_lock_by_mid();

    flash_mid_e flash_mid = flash_read_mid();
    flash_set_bank_addrs(flash_mid);

    usb_init_hid();
    irq_enable();

    while (1)
    {
        usb_poll();
    }

    return 0;
}