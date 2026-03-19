#include <stdint.h>

typedef struct __attribute__((packed))
{
    uint16_t on_time;
    uint16_t off_time;
    uint16_t count;
    uint16_t period;
    uint8_t active;
} led_dfu_t;

extern led_dfu_t led_dfu;

void ui_set_led_dfu_pattern
               (uint16_t on_ticks,uint16_t off_ticks,uint16_t blink_count,uint16_t timeout_ms);