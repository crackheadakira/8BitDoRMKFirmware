#ifndef BLE_H
#define BLE_H

#include <stdint.h>

extern uint8_t mac_public[6];
extern uint8_t mac_random_static[6];
extern uint8_t g_device_name[32];

void ble_identity_init(void);

#endif