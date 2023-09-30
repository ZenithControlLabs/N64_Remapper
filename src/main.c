#include "main.h"
#include "joybus_cons.h"
#include <stdio.h>
#include <string.h>

#include "zenith/comms/n64.h"
#include "zenith/utilities/running_avg.h"

void setup_gpio_input(uint8_t gpio) {
    gpio_init(gpio);
    // gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_IN);
}

n64_input_t _the_data;
running_avg_t x_avg;
running_avg_t y_avg;

void cb_zenith_init_hardware(void) {
    init_running_avg(&x_avg, 3, 1);
    init_running_avg(&y_avg, 3, 1);
    for (int i = 3; i < 11; i++) {
        setup_gpio_input(i);
    }
    init_joybus();
}

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
    buttons->s.b15 = gpio_get(3);
    buttons->s.b16 = gpio_get(4);
    buttons->s.b17 = gpio_get(5);
    buttons->s.b18 = gpio_get(6);
    buttons->s.b19 = gpio_get(7);
    buttons->s.b20 = gpio_get(8);
    buttons->s.b21 = gpio_get(9);
    buttons->s.b22 = gpio_get(10);
}
void cb_zenith_read_analog(analog_data_t *analog_data) {

    absolute_time_t timeout = make_timeout_time_ms(1);

    uint32_t data = read_joybus_ctlr();
    memcpy(&_the_data, &data, sizeof(n64_input_t));

    uint8_t stick_x_u = _the_data.stick_x + 128;
    uint8_t stick_y_u = _the_data.stick_y + 128;

    analog_data->ax1 = UINT_N_TO_AX(stick_x_u, 8);
    analog_data->ax2 = UINT_N_TO_AX(stick_y_u, 8);

    update_running_avg(&x_avg, stick_x_u);
    update_running_avg(&y_avg, stick_y_u);

    sleep_until(timeout);
}

void cb_zenith_read_analog_cal(analog_data_t *analog_data) {
    analog_data->ax1 = UINT_N_TO_AX(x_avg.running_sum_large >> 3, 8);
    analog_data->ax2 = UINT_N_TO_AX(y_avg.running_sum_large >> 3, 8);
}

void cb_zenith_core1_init(void) {}
void cb_zenith_core0_inject(void) {}
void cb_zenith_core1_inject(void) {}

bool cb_zenith_user_webusb_cmd(uint8_t *in, uint8_t *out) { return false; }

void cb_zenith_user_settings_reset(uint8_t *data) {}

int main() {
    set_sys_clock_khz(130000, true);

    stdio_uart_init_full(uart0, 115200, 0, -1);
    printf("N64 Remapper Started.\n");

    zenith_start();
}
