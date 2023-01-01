#include "Phobri64.h"

void second_core() {
    while (true) {
        create_default_n64_report();
        sleep_us(100);
    }
}

int main()
{
    set_sys_clock_khz(130000, true);

    multicore_lockout_victim_init();
    multicore_launch_core1(second_core);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, false);

    joybus_init_comms();

    while (1) {
        if (_report.start) {
            gpio_put(PICO_DEFAULT_LED_PIN, true);
        } else { 
            gpio_put(PICO_DEFAULT_LED_PIN, false);
        }
        sleep_ms(500);
    }

    return 0;
}

