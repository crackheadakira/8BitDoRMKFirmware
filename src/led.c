#include "stdint.h";
#include "led.h";

led_dfu_t led_dfu;

void ui_set_led_dfu_pattern(uint16_t on_ticks, uint16_t off_ticks, uint16_t blink_count, uint16_t timeout_ms)

{
    led_dfu.on_time = on_ticks;
    led_dfu.off_time = off_ticks;
    led_dfu.count = blink_count;
    led_dfu.period = timeout_ms;
    led_dfu.active = 1;
}
