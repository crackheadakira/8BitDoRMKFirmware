#include "stdint.h"
#include <stdbool.h>

void conn_mode_init();
bool is_wireless_mode(void);

typedef struct
{
    uint8_t switch_mode_b;
    uint8_t switch_mode_a;
    uint8_t _padding[2];
    uint32_t timer_1;
    uint32_t timer_2;
    uint32_t timer_3;
    uint32_t status_flags;
} conn_state_t;

typedef struct
{
    uint8_t main_mode; // 0 = USB, non-zero = BLE/2.4GHz
    uint8_t bt_active;
    uint8_t sample_idx;
    uint8_t state_flags;
} connection_mode_t;

extern conn_state_t conn_state;
extern connection_mode_t connection_mode;