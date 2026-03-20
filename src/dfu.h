/**
 * @file    dfu.h
 *
 * @brief   Header file for functions related to the DFU.
 *
 * @author  crackheadakira
 * @date    March 20, 2026
 *
 * @fw_ver  1.3.6r
 */

#pragma once
#include <stdint.h>

uint32_t dfu_hardware_init(void);
void rf_reboot(uint8_t channel);