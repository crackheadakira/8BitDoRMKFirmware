#include "drivers/B87/driver.h"

void irq_handler(void) {
}

void ble_stack_param_init(void) {
}

int main(void) {
    cpu_wakeup_init(LDO_MODE, INTERNAL_CAP_XTAL24M);
    clock_init(SYS_CLK_24M_Crystal);

    // USB init
    // usb_init();

    while(1) {
        // Poll USB — receives HID reports, dispatches to hid_flash_handler
        // usb_handle_irq();

        // TODO: matrix scan
        // TODO: send HID keyboard reports
    }

    return 0;
}