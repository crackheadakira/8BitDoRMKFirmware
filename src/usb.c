#include "register.h"
#include "stdint.h"
#include "application/usbstd/usb.h"
#include "usb.h"

static uint8_t ep_toggle = 0;
static uint8_t ep_buf[33];

uint8_t usb_ep_data_toggle[8];
uint8_t tx_packet_buffer[64];

uint8_t usb_hid_state;
uint8_t usb_connected;
uint8_t rf_tx_header_en;
uint8_t hid_seq_counter;

void usb_connection_poll()
{
    if (reg_usb_host_conn & 0x80)
    {
        if (reg_irq_src & 8)
        {
            usb_hid_state = (reg_irq_src & 0x20) ? 4 : 1;
        }
        else
        {
            usb_hid_state = 3;
        }
    }
    else
    {
        usb_hid_state = 2;
    }
}

void usb_hid_report_handler(uint8_t report_id, uint8_t len, hid_rx_packet_t *pkt)
{
    if (report_id == 0xB2 && len == 0x20)
    {
        // hid_flash_handler((uint8_t *)pkt);
    }
}

static uint32_t usb_ep_send_stateful(uint8_t endpoint, uint8_t header, uint8_t *payload, uint32_t payload_len)
{
    if ((reg_irq_src << 0x1C) < 0)
    {
        reg_usb_ep_ctrl(endpoint) = 0;
        return 1;
    }
    else if (reg_usb_host_conn == 0)
    {
        reg_usb_ep_ctrl(endpoint) = 0;
        return 2;
    }

    if (usbhw_is_ep_busy(endpoint) == 0)
    {
        // we checked if it was busy earlier
        reg_usb_ep_ptr(endpoint) = 0;

        if (rf_tx_header_en)
        {
            reg_usb_ep_ptr(endpoint + 8) = header;
        }

        uint8_t len = (uint8_t)payload_len;
        if (len > 0)
        {
            for (int i = 0; i < len; i++)
            {
                usbhw_write_ep_data(endpoint, payload[i]);
            }

            if (usb_ep_data_toggle[endpoint] == 0)
            {
                reg_usb_ep_ctrl(endpoint) = FLD_USB_EP_BUSY | FLD_USB_EP_DAT0;
            }
            else
            {
                reg_usb_ep_ctrl(endpoint) = FLD_USB_EP_BUSY | FLD_USB_EP_DAT1;
            }

            usb_ep_data_toggle[endpoint] ^= 1;

            // Success
            return 0;
        }
    }

    // Busy endpoint
    return 3;
}

void hid_send_response(uint8_t payload_len)
{
    hid_tx_packet_t *packet = (hid_tx_packet_t *)tx_packet_buffer;

    uint8_t total_len = payload_len + 2;

    packet->magic_low = 0xAA;
    packet->magic_high = 0x55;
    packet->length = total_len;
    packet->length_complement = ~total_len;
    packet->sequence_number = hid_seq_counter;

    if (hid_seq_counter == 0xFF)
    {
        hid_seq_counter = 1;
    }
    else
    {
        hid_seq_counter++;
    }

    uint16_t checksum = 0;
    if (payload_len > 0)
    {
        for (int i = 0; i < payload_len; i++)
        {
            checksum += packet->payload[i];
        }
    }

    packet->payload[payload_len] = (uint8_t)(checksum & 0xFF);
    packet->payload[payload_len + 1] = (uint8_t)((checksum >> 8) & 0xFF);

    usb_ep_send_stateful(4, 0xB1, (uint8_t *)tx_packet_buffer, 32);

    if (total_len > 27)
    {
        sleep_us(1000);
        usb_ep_send_stateful(4, 0xB1, (uint8_t *)rx_packet_buffer, 32);
    }
}

void hid_send_ready_ack(uint8_t length_complement, uint8_t command)

{
    memset(tx_packet_buffer, 0, 64);

    hid_tx_packet_t *packet = (hid_tx_packet_t *)tx_packet_buffer;

    packet->payload[0] = 0xA1;
    packet->payload[1] = length_complement;
    packet->payload[2] = command;

    hid_send_response(5);
}

void hid_send_data_nack(uint8_t param_1)

{
    memset(tx_packet_buffer, 0, 64);

    hid_tx_packet_t *packet = (hid_tx_packet_t *)tx_packet_buffer;

    packet->payload[0] = 0xA1;
    packet->payload[2] = 0x64;
    packet->payload[3] = 0x1;
    packet->payload[1] = param_1;

    hid_send_response(4);
    return;
}

void usb_hid_init(void)
{
    usbhw_disable_manual_interrupt(FLD_CTRL_EP_AUTO_STD |
                                   FLD_CTRL_EP_AUTO_DESC |
                                   FLD_CTRL_EP_AUTO_CFG);

    reg_usb_ctrl = 0x36;
    usb_connected = 0;

    gpio_set_func(GPIO_PA5, AS_USB);
    gpio_set_func(GPIO_PA6, AS_USB);

    uint8_t val = analog_read(0x0b);
    analog_write(0x0b, val | 0x7f);

    usb_hid_state = 0;

    usbhw_enable_manual_interrupt(FLD_CTRL_EP_AUTO_STD |
                                  FLD_CTRL_EP_AUTO_DESC |
                                  FLD_CTRL_EP_AUTO_CFG);
}

void usb_poll(void)

{
    usb_connection_poll();
    usb_handle_irq();
    // flash_poll();
}