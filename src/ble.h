/**
 * @file    ble.h
 *
 * @brief   Header file for functions related to BLE.
 *
 * @author  crackheadakira
 * @date    March 20, 2026
 *
 * @fw_ver  1.3.6r
 */

#pragma once
#include <stdint.h>

extern uint8_t mac_public[6];
extern uint8_t mac_random_static[6];
extern uint8_t g_device_name[32];

void ble_identity_init(void);