#include "drivers/B87/adc.h"
#include "drivers/B87/gpio.h"
#include "app_battery.h"

adc_state_t adc_state;
__attribute__((aligned(4))) uint16_t adc_dma_buf[32];

const uint8_t curve_percents[12] = {100, 90, 80, 70, 60, 50, 40, 30, 20, 10, 3, 1};
const uint16_t curve_volts_mv[12] = {4150, 4050, 3920, 3850, 3750, 3680, 3640, 3600, 3570, 3500, 3100, 1515};

uint8_t battery_level_calc(int16_t raw_adc_val)
{
    uint16_t vbatt = (raw_adc_val * 2) + 0x96;

    uint8_t final_percent = 1;

    if (vbatt > 1515)
    {

        for (uint8_t i = 0; i < 12; i++)
        {

            if (curve_volts_mv[i] <= vbatt)
            {

                final_percent = curve_percents[i];
                uint16_t v_diff = vbatt - curve_volts_mv[i];

                if ((v_diff != 0) && (i != 0))
                {

                    uint16_t p_diff = curve_percents[i - 1] - final_percent;
                    uint16_t v_step = curve_volts_mv[i - 1] - curve_volts_mv[i];

                    uint16_t added_percent = (v_diff * p_diff) / v_step;

                    if (added_percent > p_diff)
                    {
                        added_percent = p_diff;
                    }

                    final_percent += added_percent;
                }

                if (final_percent > 100)
                {
                    final_percent = 100;
                }

                break;
            }
        }
    }

    return final_percent;
}

void initialize_adc(GPIO_PinTypeDef gpio_pin, ADC_InputPchTypeDef adc_channel)
{
    adc_power_on_sar_adc(0);
    adc_enable_clk_24m_to_sar_adc(1);
    adc_set_sample_clk(5);
    adc_set_chn_enable_and_max_state_cnt(ADC_MISC_CHN, 2);
    adc_set_state_length(240, 10);

    gpio_set_func(gpio_pin, AS_GPIO);
    gpio_set_output_en(gpio_pin, 0);

    reg_gpio_pa_oen = reg_gpio_pa_oen | gpio_pin;
    reg_gpio_pa_out = reg_gpio_pa_out & ~gpio_pin;

    adc_set_ain_channel_differential_mode(adc_channel & 0xFF, GND);

    adc_set_resolution_chn_misc(RES14);

    adc_set_ref_voltage(ADC_VREF_1P2V);
    adc_set_tsample_cycle_chn_misc(SAMPLING_CYCLES_6);

    adc_set_ain_pre_scaler(ADC_PRESCALER_1F8);

    adc_power_on_sar_adc(1);

    if (adc_state.init_flag == 0)
    {
        adc_state.init_flag = 1;

        adc_state.dma_addr_1 = ((uintptr_t)adc_dma_buf) | 1;
        adc_state.dma_addr_2 = ((uintptr_t)adc_dma_buf) | 1;

        adc_state.sample_count = 4;
        adc_state.config_register = 0;

        adc_state.state_flags = 0;

        adc_state.accumulatd_val = 0;
        adc_state.filter_state = 0;

        uint32_t temp_accumulator = 0;
        for (uint8_t i = 0; i < adc_state.sample_count; i++)
        {
            uint16_t raw_val = adc_sample_and_get_result();
            temp_accumulator += raw_val;
        }

        adc_state.raw_avg = temp_accumulator / adc_state.sample_count;

        adc_state.battery_percent = battery_level_calc(adc_state.raw_avg);
        adc_state.last_sample_time = 0;
        adc_state.vbat_status = 0;
        adc_state.low_power_limit = 0;
        adc_state.vbat_mv_filtered = 0;
        adc_state.sample_interval = 0;
        adc_state.adc_mode = 0;
    }
}