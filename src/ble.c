#include "vendor/common/ble_flash.h"
#include "stack/ble/ble.h"
#include "driver_ext/ext_misc.h"
#include "ble.h"

#define DEVICE_NAME_LEN 21

uint8_t mac_public[6];
uint8_t mac_random_static[6];
uint8_t g_device_name[32];

void ble_identity_init(void)
{
    random_generator_init();

    u8 mac_address[4];
    flash_read_data(flash_sector_mac_address, 4, mac_address);

    if (*(uint32_t *)mac_address == 0xFFFFFFFF)
    {
        generateRandomNum(4, mac_address);
        flash_page_program(flash_sector_mac_address, 4, mac_address);
    }

    /*
        if (adv_channel_cfg_0 == 0xff) {
            adv_channel_cfg_0 = 0xa8;
        }
        if (adv_channel_cfg_1 == 0xff) {
            adv_channel_cfg_1 = 0x17;
        }
        if (adv_channel_cfg_2 == 0xff) {
            adv_channel_cfg_2 = 0xa8;
        }
    */

    // TODO: REPLACE
    void *device_name = 0;
    memcpy(device_name, "8BitDo Retro Keyboard", 21);
}

void ble_identity_init_incorrect(void)
{
    u8 mac_read[8];
    flash_read_data(flash_sector_mac_address, 8, mac_read);

    u8 value_rand[5];
    generateRandomNum(5, value_rand);

    u8 ff_six_byte[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    if (memcmp(mac_read, ff_six_byte, 6) != 0)
    {
        memcpy(mac_public, mac_read, 6);
    }
    else
    {
        mac_public[0] = value_rand[0];
        mac_public[1] = value_rand[1];
        mac_public[2] = value_rand[2];

        // 8BitDo's Custom OUI Fallback
        mac_public[3] = 0xA8;
        mac_public[4] = 0x17;
        mac_public[5] = 0xA8;

        flash_page_program(flash_sector_mac_address, 6, mac_public);
    }

    mac_random_static[0] = mac_public[0];
    mac_random_static[1] = mac_public[1];
    mac_random_static[2] = mac_public[2];
    mac_random_static[5] = 0xC0;

    u16 high_2_byte = (mac_read[6] | (mac_read[7] << 8));

    if (high_2_byte != 0xFFFF)
    {
        mac_random_static[3] = mac_read[6];
        mac_random_static[4] = mac_read[7];
    }
    else
    {
        mac_random_static[3] = value_rand[3];
        mac_random_static[4] = value_rand[4];

        flash_page_program(flash_sector_mac_address + 6, 2, &mac_random_static[3]);
    }

    memcpy(g_device_name, "8BitDo Retro Keyboard", DEVICE_NAME_LEN);
}