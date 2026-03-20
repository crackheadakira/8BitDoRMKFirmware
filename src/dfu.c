#include "drivers/B87/lib/include/pm.h"
#include "stdint.h"
#include "app_battery.h"
#include "connection.h"
#include "tl_snv.h"
#include "led.h"

uint8_t dfu_active;
uint8_t rf_channel;
adc_state_t adc_state;

uint32_t flash_timeout_1;

uint32_t flash_set_timeout(int timeout)

{
    uint32_t result;

    result = 0xD0;

    if ((timeout - 2) < 0xF9)
    {
        flash_timeout_1 = timeout * 1000000;
        result = 0;
    }

    return result;
}

uint32_t dfu_hardware_init(void)

{

    dfu_active = 0x1;

    if (is_wireless_mode())
    {
        if (10 < adc_state.battery_percent)
        {
            flash_unlock_by_mid();
        }
    }
    else
    {
        flash_unlock_by_mid();
    }

    led_set_dfu_pattern(6, 6, 0, 300);

    flash_set_timeout(1000);

    return 1;
}

void set_rf_channel(uint8_t channel)
{
    rf_channel = channel;
    analog_write(DEEP_ANA_REG0, channel);
    return;
}

void rf_reboot(uint8_t channel)
{
    set_rf_channel(channel);
    start_reboot();
    return;
}