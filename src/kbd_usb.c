#include "application/usbstd/usb.h"
#include "connection.h"
#include "drivers/B87/register.h"

#include "app_config.h"
#include "dfu.h"
#include "kbd_usb.h"
#include "tl_snv.h"

typedef struct
{
    u8 hid_state;
    u8 pad_1;
    u8 pad_2;
    u8 pad_3;
    u32 send_timer;
    u8 conn_state;
    u8 pad_4;
    u8 pad_5;
    u8 pad_6;
    u32 send_last_tick;
    u32 disconnect_timer;
    u32 send_timestamp;
} hid_state_t;

const USB_Descriptor_Device_t device_desc = {
    .Header = {
        .Size = sizeof(USB_Descriptor_Device_t),
        .Type = DTYPE_Device,
    },
    .USBSpecification = 0x0110,
    .Class = 0x00,
    .SubClass = 0x00,
    .Protocol = 0x00,
    .Endpoint0Size = 0x08,
    .VendorID = 0x2DC8,
    .ProductID = 0x5200,
    .ReleaseNumber = 0x0100,
    .ManufacturerStrIndex = 0x01,
    .ProductStrIndex = 0x02,
    .SerialNumStrIndex = 0x03,
    .NumberOfConfigurations = 0x01,
};

const USB_Descriptor_Configuration_t configuration_desc = {
    .Config = {
        .Header = {
            .Size = sizeof(USB_Descriptor_Configuration_Hdr_t),
            .Type = DTYPE_Configuration,
        },
        .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
        .TotalInterfaces = 3,
        .ConfigurationNumber = 1,
        .ConfigurationStrIndex = 0,
        .ConfigAttributes = 0xA0,
        .MaxPowerConsumption = 0x64,
    },

    // --- Interface 0: Mouse (EP2 IN) ---
    .mouse_interface = {
        .Header = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
        .InterfaceNumber = 0,
        .AlternateSetting = 0,
        .TotalEndpoints = 1,
        .Class = HID_CSCP_HIDClass,
        .SubClass = HID_CSCP_BootSubclass,
        .Protocol = HID_CSCP_MouseBootProtocol,
        .InterfaceStrIndex = NO_DESCRIPTOR,
    },
    .mouse_descriptor = {
        .mouse_hid = {
            .Header = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
            .HIDSpec = 0x0111,
            .CountryCode = 0x21,
            .TotalReportDescriptors = 1,
            .HIDReportType = HID_DTYPE_Report,
            .HIDReportLength = {79},
        },
        .mouse_in_endpoint = {
            .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
            .EndpointAddress = 0x82,
            .Attributes = 0x03,
            .EndpointSize = 0x0040,
            .PollingIntervalMS = 1,
        },
    },

    // --- Interface 1: Keyboard (EP1 IN) ---
    .keyboard_interface = {
        .Header = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
        .InterfaceNumber = 1,
        .AlternateSetting = 0,
        .TotalEndpoints = 1,
        .Class = HID_CSCP_HIDClass,
        .SubClass = HID_CSCP_BootSubclass,
        .Protocol = HID_CSCP_KeyboardBootProtocol,
        .InterfaceStrIndex = NO_DESCRIPTOR,
    },
    .keyboard_descriptor = {
        .keyboard_hid = {
            .Header = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
            .HIDSpec = 0x0111,
            .CountryCode = 0x21,
            .TotalReportDescriptors = 1,
            .HIDReportType = HID_DTYPE_Report,
            .HIDReportLength = {245},
        },
        .keyboard_in_endpoint = {
            .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
            .EndpointAddress = 0x81,
            .Attributes = 0x03,
            .EndpointSize = 0x0040,
            .PollingIntervalMS = 1,
        },
    },

    // --- Interface 2: Vendor/Somatic (EP4 IN + EP5 OUT) ---
    .somatic_interface = {
        .Header = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},
        .InterfaceNumber = 2,
        .AlternateSetting = 0,
        .TotalEndpoints = 2,
        .Class = HID_CSCP_HIDClass,
        .SubClass = HID_CSCP_BootSubclass,
        .Protocol = HID_CSCP_MouseBootProtocol,
        .InterfaceStrIndex = NO_DESCRIPTOR,
    },
    .somatic_descriptor = {
        .somatic_hid = {
            .Header = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},
            .HIDSpec = 0x0111,
            .CountryCode = 0x21,
            .TotalReportDescriptors = 1,
            .HIDReportType = HID_DTYPE_Report,
            .HIDReportLength = {82},
        },
        .somatic_in_endpoint = {
            .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
            .EndpointAddress = 0x84,
            .Attributes = 0x03,
            .EndpointSize = 0x0040,
            .PollingIntervalMS = 1,
        },
        .somatic_out_endpoint = {
            .Header = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
            .EndpointAddress = 0x05,
            .Attributes = 0x03,
            .EndpointSize = 0x0040,
            .PollingIntervalMS = 1,
        },
    },
};

const USB_Descriptor_String_t language_desc = {
    .Header = {
        .Size = sizeof(USB_Descriptor_Hdr_t) + 2,
        .Type = DTYPE_String},
    .UnicodeString = {LANGUAGE_ID_ENG}};

const USB_Descriptor_String_t vendor_desc = {
    .Header = {
        .Size = sizeof(USB_Descriptor_Hdr_t) + sizeof(STRING_VENDOR) - 2,
        .Type = DTYPE_String},
    .UnicodeString = STRING_VENDOR};

const USB_Descriptor_String_t product_desc = {
    .Header = {
        .Size = sizeof(USB_Descriptor_Hdr_t) + sizeof(STRING_PRODUCT) - 2,
        .Type = DTYPE_String},
    .UnicodeString = {STRING_PRODUCT}};

const USB_Descriptor_String_t serial_desc = {
    .Header = {
        .Size = sizeof(USB_Descriptor_Hdr_t) + sizeof(STRING_SERIAL) - 2,
        .Type = DTYPE_String},
    .UnicodeString = {STRING_SERIAL}};

uint8_t usb_ep_data_toggle[8];
uint8_t tx_packet_buffer[64];

hid_state_t usb_state;
uint8_t usb_connected;
uint8_t rf_tx_header_en;
uint8_t hid_seq_counter;
uint8_t target_bank_id;

uint8_t is_dfu_initialized;
uint8_t is_receiving_data;
uint8_t is_end_of_file;

uint32_t current_flash_offset;
uint16_t packet_counter;
uint32_t running_crc_bank0;
uint32_t running_crc_bank1;

static uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        uint32_t byte = (data[i] ^ crc) & 0xFF;
        for (int bit = 0; bit < 8; bit++)
        {
            if (byte & 1)
            {
                byte = (byte >> 1) ^ 0xEDB88320;
            }
            else
            {
                byte >>= 1;
            }
        }
        crc = (crc >> 8) ^ byte;
    }
    return crc;
}

void usb_connection_poll()
{
    uint32_t irq_src = read_reg32(reg_irq_src);
    uint8_t connection_state = 2;

    if (read_reg8(reg_usb_host_conn) & 0x80)
    {
        connection_state = 3;
        if ((irq_src & 8) != 0)
        {
            if (read_reg8(reg_usb_mdev) << 0x1D > -1)
            {
                connection_state = 4;
            }
            else
            {
                connection_state = 1;
            }
        }
        else
        {
            if (!is_bluetooth_active())
            {
                usb_state.conn_state = 1;
            }
            else
            {
                usb_state.conn_state = 4;
                /*
                (*(code *)((int)&USER_NMI + 2))(0,0);
                rf_2g4_channel_init();
                rf_2g4_sync_init();
                usb_poll();
                sleep_us(1000);
                usb_poll();
                rf_2g4_status_check();
                keyboard_cols_pulse();
                noop();
                */
            }
        }
    }

    if (usb_state.conn_state > 2)
    {
        if ((irq_src & 8) == 0 && usb_state.disconnect_timer != 0)
        {
            usb_state.disconnect_timer = 0;
        }
        else if (usb_state.disconnect_timer == 0)
        {
            usb_state.disconnect_timer = read_reg32(reg_system_tick) | 1;
        }
        else if (12800000 < read_reg32(reg_system_tick) - usb_state.disconnect_timer)
        {
            usb_state.disconnect_timer = 0;
            if (!is_bluetooth_active())
            {
                usb_state.conn_state = 1;
            }
        }
    }

    if (connection_state != usb_state.conn_state)
    {
        usb_state.conn_state = connection_state;
    }
}

static uint32_t usb_ep_send_stateful(uint8_t endpoint, uint8_t header,
                                     uint8_t *payload, uint32_t payload_len)
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

void hid_send_ready_ack(uint8_t sequence_number, uint8_t command)

{
    memset(tx_packet_buffer, 0, 64);

    hid_tx_packet_t *packet = (hid_tx_packet_t *)tx_packet_buffer;

    packet->payload[0] = 0xA1;
    packet->payload[1] = sequence_number;
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

static uint32_t verify_firmware_image(void)
{
    uint32_t firmware_buf;
    flash_read_page(ota_program_offset + 24, 4, (uint8_t *)&firmware_buf);

    uint32_t num_chunks = (firmware_buf << 8) >> 16;
    uint32_t crc = 0xFFFFFFFE;

    uint8_t chunk[0x100];

    for (int i = 0; i < num_chunks; i++)
    {
        flash_read_page(ota_program_offset + 0x100 * i, 0x100, chunk);

        if ((i == 0) && (chunk[8] == 0xFF))
        {
            chunk[8] = 'K';
        }

        crc = crc32_update(crc, chunk, 0x100);
    }

    uint32_t remainder = firmware_buf & 0xFF;
    if (remainder != 0)
    {
        flash_read_page(ota_program_offset + firmware_buf - remainder, remainder,
                        chunk);
        crc = crc32_update(crc, chunk, remainder);
    }

    return crc & 0xFF;
}

static void handle_query_device_info_cmd(hid_rx_packet_t *packet)
{
    memset(&tx_packet_buffer, 0, 64);
    hid_tx_packet_t *tx_packet = (hid_tx_packet_t *)tx_packet_buffer;

    tx_packet->payload[0] = 0xA3;
    tx_packet->payload[1] = packet->sequence_number;
    tx_packet->payload[2] = packet->command;
    tx_packet->payload[3] = 0x01;

    tx_packet->payload[0x0E] = 0x05;
    tx_packet->payload[0x0F] = 0x03;

    memcpy(&tx_packet->payload[4], FIRMWARE_MAGIC_NAME, 8);
    memcpy(&tx_packet->payload[16], FIRMWARE_VERSION_LONG, 6);
    memcpy(&tx_packet->payload[26], FIRMWARE_VERSION_SHORT, 5);

    hid_send_response(0x26);
}

static void handle_cmd_get_capabilities(hid_rx_packet_t *packet)
{
    memset(&tx_packet_buffer, 0, 64);
    hid_tx_packet_t *tx_packet = (hid_tx_packet_t *)tx_packet_buffer;

    tx_packet->payload[4] = 0x1;
    tx_packet->payload[0] = 0xA3;
    tx_packet->payload[1] = packet->sequence_number;
    tx_packet->payload[2] = packet->command;

    hid_send_response(0x7);
}

static void handle_cmd_enter_dfu_mode(hid_rx_packet_t *packet)
{
    memset(&tx_packet_buffer, 0, 0x40);
    hid_tx_packet_t *tx_packet = (hid_tx_packet_t *)tx_packet_buffer;

    if (packet->data[0] < 0x3)
    {
        target_bank_id = packet->data[0];
    }

    tx_packet->payload[0] = 0xA3;
    tx_packet->payload[2] = 0x62;
    tx_packet->payload[3] = ~-(2 < packet->data[0]);
    tx_packet->payload[1] = packet->sequence_number;
    hid_send_response(0x9);
}

static void handle_cmd_confirm_ready(uint8_t sequence_number)
{
    dfu_hardware_init();

    is_dfu_initialized = 0x1;
    current_flash_offset = 0x0;
    packet_counter = 0x0;
    is_end_of_file = 0x0;
    running_crc_bank0 = 0xfffffffe;
    running_crc_bank1 = 0xfffffffe;

    hid_send_ready_ack(sequence_number, 0x63);
}

static void handle_cmd_data(hid_rx_packet_t *packet)
{
    is_receiving_data = 1;
    if (is_dfu_initialized == 0)
    {
        hid_send_data_nack(packet->sequence_number);
        return;
    }

    uint8_t data_len = packet->length - 4;
    uint32_t write_len = data_len;

    if (target_bank_id == 0)
    {
        running_crc_bank0 =
            crc32_update(running_crc_bank0, packet->data, data_len);
    }

    // TOOD: add bank 1 support

    if (is_end_of_file == 0)
    {
        if ((current_flash_offset & 0xFFF) == 0)
        {
            flash_erase_sector(ota_program_offset + current_flash_offset);
        }

        if (data_len == 16 && packet_counter < 0x3FFF)
        {
            if ((packet_counter == 0) && (packet->data[8] == 'K'))
            {
                packet->data[8] = 0xFF;
            }

            flash_page_program(ota_program_offset + current_flash_offset, 16,
                               packet->data);
            packet_counter += 1;
        }
        else if ((data_len == 4) && (packet_counter < 0xFFF))
        {
            flash_page_program(ota_program_offset + current_flash_offset, 4,
                               packet->data);
            packet_counter += 1;
            is_end_of_file = 1;
        }

        current_flash_offset += write_len;
        hid_send_ready_ack(packet->sequence_number, 100);
        return;
    }
    current_flash_offset += write_len;
}

static void handle_cmd_commit(hid_rx_packet_t *packet)
{
    memset(&tx_packet_buffer, 0, 64);
    hid_tx_packet_t *tx_packet = (hid_tx_packet_t *)tx_packet_buffer;

    uint32_t crc_received =
        (uint32_t)packet->data[3] << 24 | (uint32_t)packet->data[0] |
        (uint32_t)packet->data[1] << 8 | (uint32_t)packet->data[2] << 16;

    uint32_t crc_expected =
        (uint32_t)packet->data[7] << 24 | (uint32_t)packet->data[4] |
        (uint32_t)packet->data[5] << 8 | (uint32_t)packet->data[6] << 16;

    if (target_bank_id == 0)
    {
        tx_packet->payload[4] = true;
        if (crc_received == crc_expected)
        {
            tx_packet->payload[4] = crc_received != running_crc_bank0;
        }
    }
    else
    {
        tx_packet->payload[4] = true;
        if ((target_bank_id == 1) && (crc_received == running_crc_bank1))
        {
            tx_packet->payload[4] = crc_expected != running_crc_bank0;
        }
    }
    tx_packet->payload[0] = 0xA3;
    tx_packet->payload[2] = 0x65;
    tx_packet->payload[3] = 0x00;
    tx_packet->payload[1] = packet->sequence_number;

    hid_send_response(5);
}

static void handle_cmd_reboot(hid_rx_packet_t *packet)
{
    if (is_end_of_file == 0)
        return;

    if (verify_firmware_image() == 0)
    {
        uint8_t knlt_byte = 'K';
        uint8_t zero_byte = 0;

        flash_page_program(ota_program_offset + 8, 1, &knlt_byte);

        uint32_t boot_addr = 8;
        if (ota_program_offset == 0)
        {
            boot_addr = ota_program_bootAddr + 8;
        }

        flash_page_program(boot_addr, 1, &zero_byte);
        flash_lock_by_mid();

        sleep_us(10000);
        sleep_us(100000);
        rf_reboot(0);
    }
    else
    {
        sleep_us(100000);
        rf_reboot(0);
    }
}

void hid_handle_flash(hid_rx_packet_t *packet)
{
    uint16_t magic = (uint32_t)((packet->magic_high << 8) | packet->magic_low);

    if (magic != 0xAA55 && magic != 0xAA56)
    {
        return;
    }

    if (packet->length != (uint8_t)~packet->length_complement)
    {
        return;
    }

    if (packet->length <= 2)
    {
        return;
    }

    uint16_t checksum_accum = packet->command;
    uint8_t loop_limit = packet->length - 3;

    for (uint8_t i = 0; i < loop_limit; i++)
    {
        checksum_accum += packet->data[i];
    }

    if (packet->crc != checksum_accum)
    {
        return;
    }

    switch (packet->command)
    {
    case QUERY_DEVICE_INFO:
        handle_query_device_info_cmd(packet);
        break;

    case GET_CAPABILITIES:
        handle_cmd_get_capabilities(packet);
        break;

    case ENTER_DFU_MODE:
        handle_cmd_enter_dfu_mode(packet);
        break;

    case CONFIRM_READY:
        if (magic == 0xAA56)
            handle_cmd_confirm_ready(packet->sequence_number);
        break;

    case DATA:
        handle_cmd_data(packet);
        break;

    case COMMIT:
        handle_cmd_commit(packet);
        break;

    case REBOOT:
        handle_cmd_reboot(packet);
        break;

    case DEBUG:
        break;
    }
}

void usb_init_hid(void)
{
    usbhw_enable_manual_interrupt(0xA0);
    usbhw_set_eps_en(0x36);

    usb_connected = 0;

    gpio_set_func(GPIO_PA5, AS_USB);
    gpio_set_func(GPIO_PA6, AS_USB);

    analog_write(reg_spi_inv_clk, analog_read(reg_spi_inv_clk) | 0x7F);
    usb_state.hid_state = 0;
    usb_state.pad_1 = 0;
    usb_state.pad_2 = 0;
    usb_state.pad_3 = 0;
    usb_state.send_timer = 0;
}

uint8_t *usbdesc_get_device(void)
{
    return (uint8_t *)(&device_desc);
}

uint8_t *usbdesc_get_serial(void)
{
    return (uint8_t *)(&serial_desc);
}

uint8_t *usbdesc_get_configuration(void)
{
    return (uint8_t *)(&configuration_desc);
}

uint8_t *usbdesc_get_language(void)
{
    return (uint8_t *)(&language_desc);
}

uint8_t *usbdesc_get_vendor(void)
{
    return (uint8_t *)(&vendor_desc);
}

uint8_t *usbdesc_get_product(void)
{
    return (uint8_t *)(&product_desc);
}

void usb_hid_report_handler(uint8_t report_id, uint8_t len,
                            hid_rx_packet_t *pkt)
{
    if (report_id == 0xB2 && len == 0x20)
    {
        hid_handle_flash(pkt);
    }
}

void usb_irq_handler(void)
{
    // TODO: fill out
    usb_handle_irq();
}

void usb_poll(void)

{
    usb_connection_poll();
    usb_irq_handler();
    // flash_poll();
}