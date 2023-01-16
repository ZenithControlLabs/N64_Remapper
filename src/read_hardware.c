#include "Phobri64.h"

inline float read_stick_x() {
    /*
    adc_select_input(STICK_X_ADC);
    return adc_read() / ADC_MAX;
    */
    return 0;
}

inline float read_stick_y() {
    /*
    adc_select_input(STICK_Y_ADC);
    return adc_read() / ADC_MAX;
    */
    return 0;
}

static inline void init_btn_pin(uint pin) {
    gpio_init(pin);
	gpio_pull_up(pin);
	gpio_set_dir(pin, GPIO_IN);
}

void init_hardware() {
    /*
    init_btn_pin(BTN_A_PIN);
    init_btn_pin(BTN_B_PIN);
    init_btn_pin(BTN_START_PIN);
    init_btn_pin(BTN_R_PIN);
    init_btn_pin(BTN_L_PIN);
    init_btn_pin(BTN_ZR_PIN);
    init_btn_pin(BTN_ZL_PIN);
    init_btn_pin(BTN_CR_PIN);
    init_btn_pin(BTN_CL_PIN);
    init_btn_pin(BTN_CD_PIN);
    init_btn_pin(BTN_CU_PIN);
    init_btn_pin(BTN_DR_PIN);
    init_btn_pin(BTN_DL_PIN);
    init_btn_pin(BTN_DD_PIN);
    init_btn_pin(BTN_DU_PIN);

    adc_init(STICK_X_ADC);
    adc_init(STICK_Y_ADC);

    adc_gpio_init(STICK_X_PIN);
    adc_gpio_init(STICK_Y_PIN);
    */
    return;
}

raw_report_t read_hardware() {
    raw_report_t report = {
        .a = 0,
        .b = 0,
        .start = 0,
        .r = 0,
        .l = 0,
        .zr = 0,
        .zl = 0,
        .reserved0 = 0,
        .c_right = 0,
        .c_left = 0,
        .c_up = 0,
        .dpad_right = 0,
        .dpad_left = 0,
        .dpad_down = 0,
        .dpad_up = 0,
        .stick_x = 0.0f,
        .stick_y = 0.0f,
    };
    /*
    report.stick_x = read_stick_x();
    report.stick_y = read_stick_y();

    report.a = !gpio_get(BTN_A_PIN);
    report.b = !gpio_get(BTN_B_PIN);
    report.start = !gpio_get(BTN_START_PIN);
    report.r = !gpio_get(BTN_R_PIN);
    report.l = !gpio_get(BTN_L_PIN);
    report.zr = !gpio_get(BTN_ZR_PIN);
    report.zl = !gpio_get(BTN_ZL_PIN);
    report.c_right = !gpio_get(BTN_CR_PIN);
    report.c_left = !gpio_get(BTN_CL_PIN);
    report.c_down = !gpio_get(BTN_CD_PIN);
    report.c_up = !gpio_get(BTN_CU_PIN);
    report.dpad_right = !gpio_get(BTN_DR_PIN);
    report.dpad_left = !gpio_get(BTN_DL_PIN);
    report.dpad_down = !gpio_get(BTN_DD_PIN);
    report.dpad_up = !gpio_get(BTN_DU_PIN);
    */
    
    return report;
}