#include <stdint.h>
#include <pm.h>

typedef enum
{
    QUERY_DEVICE_INFO = 0x60,
    GET_CAPABILITIES = 0x61,
    ENTER_DFU_MODE = 0x62,
    CONFIRM_READY = 0x63,
    DATA = 0x64,
    COMMIT = 0x65,
    REBOOT = 0x66,
    DEBUG = 0xF2,
} hid_command_t;

typedef struct __attribute__((packed))
{
    uint8_t magic_high;
    uint8_t magic_low;
    uint8_t length;
    uint8_t length_complement;
    uint8_t sequence_number;
    hid_command_t command;
    uint8_t data[16];
    uint16_t crc;
    uint8_t padding[8];
} hid_rx_packet_t;

typedef struct __attribute__((packed))
{
    uint8_t magic_high;
    uint8_t magic_low;
    uint8_t length;
    uint8_t length_complement;
    uint8_t sequence_number;
    uint8_t payload[];
} hid_tx_packet_t;

extern uint8_t hid_seq_counter;
extern uint8_t usb_hid_state;
extern uint8_t usb_connected;
extern uint8_t rf_tx_header_en;
extern uint8_t target_bank_id;

extern uint8_t is_dfu_initialized;
extern uint8_t is_receiving_data;
extern uint8_t is_end_of_file;

extern uint32_t current_flash_offset;
extern uint16_t packet_counter;
extern uint32_t running_crc_bank0;
extern uint32_t running_crc_bank1;

extern uint8_t usb_ep_data_toggle[8];

extern uint8_t tx_packet_buffer[64];
#define rx_packet_buffer (tx_packet_buffer + 32)

void usb_hid_init(void);
void usb_poll(void);
uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t length);