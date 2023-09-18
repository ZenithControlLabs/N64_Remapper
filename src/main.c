#include "main.h"
#include "joybus_cons.h"
#include <stdio.h>

#include "zenith/comms/n64.h"

void setup_gpio_button(uint8_t gpio) {
    gpio_init(gpio);
    gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_IN);
}

n64_input_t _the_data;

void cb_zenith_init_hardware(void) { init_joybus(); }
void cb_zenith_read_buttons(btn_data_t *buttons) {
    buttons->s.b1 = _the_data.button_a;
    buttons->s.b2 = _the_data.button_b;
    buttons->s.b3 = _the_data.cpad_up;
    buttons->s.b4 = _the_data.cpad_down;
    buttons->s.b5 = _the_data.cpad_left;
    buttons->s.b6 = _the_data.cpad_right;
    buttons->s.b7 = _the_data.button_start;
    buttons->s.b8 = _the_data.button_l;
    buttons->s.b9 = _the_data.button_r;
    buttons->s.b10 = _the_data.button_z;
    buttons->s.b11 = _the_data.dpad_down;
    buttons->s.b12 = _the_data.dpad_left;
    buttons->s.b13 = _the_data.dpad_right;
    buttons->s.b14 = _the_data.dpad_up;
}
void cb_zenith_read_analog(analog_data_t *analog_data) {

    absolute_time_t timeout = make_timeout_time_ms(1);

    uint32_t data = read_joybus_ctlr();
    memcpy(&_the_data, &data, sizeof(n64_input_t));

    analog_data->ax1 = INT_N_TO_AX(_the_data.stick_x, 8);
    analog_data->ax2 = INT_N_TO_AX(_the_data.stick_y, 8);
    // printf("b=%d\n", data);

    sleep_until(timeout);
}

void cb_zenith_core1_init(void) {}
void cb_zenith_core0_inject(void) {}
void cb_zenith_core1_inject(void) {}

bool cb_zenith_user_webusb_cmd(uint8_t *in, uint8_t *out) { return false; }

void cb_zenith_user_settings_reset(uint8_t *data) {}

int main() {
    set_sys_clock_khz(130000, true);

    stdio_uart_init_full(uart0, 115200, 12, -1);
    printf("N64 Remapper Started.\n");

    zenith_start();
}
