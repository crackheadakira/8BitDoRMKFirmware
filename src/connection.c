#include "drivers/B87/gpio.h"
#include "connection.h"

conn_state_t conn_state;
uint32_t conn_ready_flag = 0;

void conn_mode_init(void)
{
    uint8_t pd1_is_high = gpio_read(GPIO_PD1);
    uint8_t pd2_is_high = gpio_read(GPIO_PD2);

    if (pd1_is_high)
    {
        conn_state.switch_mode_a = 0;
    }
    else
    {
        conn_state.switch_mode_a = 1;
    }

    uint8_t mode_b_val = 1;

    if (pd1_is_high)
    {
        if (pd2_is_high)
        {
            mode_b_val = 0;
        }
        else
        {
            mode_b_val = 254;
        }
    }

    conn_state.switch_mode_b = mode_b_val;

    conn_state.timer_1 = 0;
    conn_state.timer_2 = 0;
    conn_state.timer_3 = 0;

    conn_state.status_flags = conn_ready_flag | 1;
}

bool is_wireless_mode(void)
{
    return connection_mode.main_mode != 0;
}