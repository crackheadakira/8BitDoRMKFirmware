#include "stdint.h"

void conn_mode_init();

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

extern conn_state_t conn_state;
extern volatile uint32_t connection_mode;