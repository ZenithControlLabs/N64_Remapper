#include "Phobri64.h"

bool _please_commit = false;

void second_core() {
    create_default_n64_report();

    debug_print("Phobri64 Initialization\n");
    while (true) {
        if (_please_commit) {
            commit_config_state();
            _please_commit = false;
        }

        process_controller();

        sleep_us(100);
    }
}

int main() {
    set_sys_clock_khz(130000, true);
    multicore_lockout_victim_init();

    // Initialize all relevant hardware pins
    stdio_uart_init_full(uart0, 115200, DEBUG_TX_PIN, -1);
    init_hardware();

    // Initialize report mutex before launching things on both cores
    mutex_init(&_report_lock);

    // Startup checks
    raw_report_t r_report = read_hardware(true);
    // reboot in BOOTSEL mode if start is held
    if (r_report.start) {
        sleep_ms(1);
        reset_usb_boot(0, 0);
    }

    // Initialize config state (load stuff from flash)
    init_config_state();

    // Now launch core1
    multicore_launch_core1(second_core);

    joybus_init_comms();
    usb_init_comms();

    while (1) {
        usb_run_comms();
    }

    return 0;
}
