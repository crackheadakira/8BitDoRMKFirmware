#include <stdint.h>

typedef struct __attribute__((packed))
{
    uint8_t magic_high;
    uint8_t magic_low;
    uint8_t length;
    uint8_t length_complement;
    uint8_t sequence_number;
    uint8_t command;
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

extern uint8_t usb_ep_data_toggle[8];

extern uint8_t tx_packet_buffer[64];
#define rx_packet_buffer (tx_packet_buffer + 32)

void usb_hid_init(void);
void usb_poll(void);