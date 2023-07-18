#include "main.h"

bool _please_commit = false;

void second_core() {
    create_default_n64_report();

    debug_print("N64 remapper Initialization\n");

    init_hardware();

    debug_print("Successfully initialized joystick\n");

    while (true) {
        if (_please_commit) {
            commit_config_state();
            _please_commit = false;
        }

        process_controller();

        sleep_ms(1);
    }
}

int main() {
    set_sys_clock_khz(130000, true);
    multicore_lockout_victim_init();

    // Initialize all relevant hardware pins
    stdio_uart_init_full(uart0, 115200, DEBUG_TX_PIN, -1);

    // Initialize report mutex before launching things on both cores
    mutex_init(&_report_lock);

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
