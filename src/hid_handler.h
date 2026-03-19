#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t inv_length;
    uint8_t magic_high;
    uint8_t magic_low;
    uint8_t command;
    uint8_t data[25];
    uint16_t checksum;
} dfu_packet_t;

enum {
    QUERY_DEVICE_INFO = 0x60,
    GET_CAPABILITIES = 0x61,
    ENTER_DFU_MODE = 0x62,
    CONFIRM_READY = 0x63,
    DATA = 0x64,
    COMMIT = 0x65,
    REBOOT = 0x66
} hid_command;