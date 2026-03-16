// app_battery.h
#ifndef APP_BATTERY_H
#define APP_BATTERY_H

#include <stdint.h>
#include <gpio.h>
#include <adc.h>

typedef struct
{
    uint32_t state_flags;
    uint32_t config_register;
    uint32_t accumulatd_val;
    uint32_t filter_state;
    uint8_t sample_count;
    uint8_t _padding_1;
    int16_t raw_avg;
    uint8_t init_flag;
    uint8_t _padding_2[3];
    uint32_t dma_addr_1;
    uint32_t dma_addr_2;
    uint8_t battery_percent;
    uint8_t _padding_3[3];
    uint32_t last_sample_time;
    uint8_t vbat_status;
    uint8_t _padding_4[3];
    uint32_t low_power_limit;
    uint16_t vbat_mv_filtered;
    uint8_t _padding_5[2];
    uint32_t sample_interval;
    uint8_t adc_mode;
    uint8_t _padding_6[3];
} adc_state_t;

extern adc_state_t adc_state;

uint8_t battery_level_calc(int16_t raw_adc_val);
void initialize_adc(GPIO_PinTypeDef gpio_pin, ADC_InputPchTypeDef adc_channel);

#endif