#include "Phobri64.h"

mutex_t adc_mtx;

// for external MCP3202 adc, 12 bit
uint16_t __time_critical_func(read_ext_adc)(bool isXaxis) {
    mutex_enter_blocking(&adc_mtx);
    const uint8_t config_val = isXaxis ? 0xD0 : 0xF0;
    uint8_t data_buf[3];
    gpio_put(STICK_SPI_CS, 0);

    spi_read_blocking(spi0, config_val, data_buf, 3);
    uint16_t tempValue =
        ((data_buf[0] & 0x07) << 9) | data_buf[1] << 1 | data_buf[2] >> 7;

    gpio_put(STICK_SPI_CS, 1);
    mutex_exit(&adc_mtx);
    return tempValue;
}

inline float read_stick_x() { return read_ext_adc(true) / ADC_MAX; }

inline float read_stick_y() { return read_ext_adc(false) / ADC_MAX; }

static inline void init_btn_pin(uint pin) {
    gpio_init(pin);
    gpio_pull_up(pin);
    gpio_set_dir(pin, GPIO_IN);
}

void init_hardware() {

    init_btn_pin(BTN_A_PIN);
    init_btn_pin(BTN_B_PIN);
    init_btn_pin(BTN_START_PIN);
    init_btn_pin(BTN_R_PIN);
    init_btn_pin(BTN_L_PIN);
    // init_btn_pin(BTN_ZR_PIN);
    init_btn_pin(BTN_ZL_PIN);
    init_btn_pin(BTN_CR_PIN);
    init_btn_pin(BTN_CL_PIN);
    // init_btn_pin(BTN_CD_PIN);
    // init_btn_pin(BTN_CU_PIN);
    init_btn_pin(BTN_DR_PIN);
    init_btn_pin(BTN_DL_PIN);
    init_btn_pin(BTN_DD_PIN);
    init_btn_pin(BTN_DU_PIN);

    // 3MHz
    spi_init(spi0, 3000 * 1000);
    gpio_set_function(STICK_SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(STICK_SPI_TX, GPIO_FUNC_SPI);
    gpio_set_function(STICK_SPI_RX, GPIO_FUNC_SPI);
    gpio_init(STICK_SPI_CS);
    gpio_set_dir(STICK_SPI_CS, GPIO_OUT);
    gpio_put(STICK_SPI_CS, 1); // active low
#ifdef CSTICK_SPI_CS
    // phob2 devboard has a c-stick SPI interface as well
    // leave chip select high so it doesnt interfere with us
    gpio_init(CSTICK_SPI_CS);
    gpio_set_dir(CSTICK_SPI_CS, GPIO_OUT);
    gpio_put(CSTICK_SPI_CS, 1);
#endif

    mutex_init(&adc_mtx);
    return;
}

raw_report_t read_hardware(bool quick) {
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

    if (!quick) {
        report.stick_x = read_stick_x();
        report.stick_y = read_stick_y();
    }

    report.a = !gpio_get(BTN_A_PIN);
    report.b = !gpio_get(BTN_B_PIN);
    report.start = !gpio_get(BTN_START_PIN);
    report.r = !gpio_get(BTN_R_PIN);
    report.l = !gpio_get(BTN_L_PIN);
    // report.zr = !gpio_get(BTN_ZR_PIN);
    report.zl = !gpio_get(BTN_ZL_PIN);
    report.c_right = !gpio_get(BTN_CR_PIN);
    report.c_left = !gpio_get(BTN_CL_PIN);
    report.dpad_right = !gpio_get(BTN_DR_PIN);
    report.dpad_left = !gpio_get(BTN_DL_PIN);
    report.dpad_down = !gpio_get(BTN_DD_PIN);
    report.dpad_up = !gpio_get(BTN_DU_PIN);

    return report;
}