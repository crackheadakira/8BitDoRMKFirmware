/**
 * @file    led.h
 *
 * @brief   Header file for functions related to LEDs.
 *
 * @author  crackheadakira
 * @date    March 20, 2026
 *
 * @fw_ver  1.3.6r
 */

#pragma once
#include <stdint.h>

typedef struct __attribute__((packed))
{
    uint16_t on_time;
    uint16_t off_time;
    uint16_t count;
    uint16_t period;
    uint8_t active;
} led_dfu_t;

/**
 * @brief   Sets `led_dfu_t` to given parameters.
 *
 * @note    Also sets led_dfu_t.active to 1.
 *
 * @param   on_ticks    How many ticks should the light be on for.
 * @param   off_ticks   How many ticks should the light be off for.
 * @param   blink_count How many times to blink the on-off cycle.
 * @param   timeout_ms  Unsure.
 *
 * @fw_addr 0x00016814
 * @fw_ver  1.3.6r
 * @source  re
 */
void led_set_dfu_pattern(uint16_t on_ticks, uint16_t off_ticks, uint16_t blink_count, uint16_t timeout_ms);