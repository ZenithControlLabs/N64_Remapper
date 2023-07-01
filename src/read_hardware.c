#include "Phobri64.h"

mutex_t adc_mtx;

// for external MCP3202 adc, 12 bit
uint16_t __time_critical_func(read_ext_adc)(axis_t which_axis) {
    mutex_enter_blocking(&adc_mtx);
    const uint8_t config_val =
        (STICK_FLIP_ADC_CHANNELS ^ (which_axis == XAXIS)) ? 0xD0 : 0xF0;
    uint8_t data_buf[3];
    gpio_put(STICK_SPI_CS, 0);

    spi_read_blocking(STICK_SPI_INTF, config_val, data_buf, 3);
    uint16_t tempValue =
        ((data_buf[0] & 0x07) << 9) | data_buf[1] << 1 | data_buf[2] >> 7;

    gpio_put(STICK_SPI_CS, 1);
    mutex_exit(&adc_mtx);
    return tempValue;
}

raw_stick_t read_stick_multisample() {
    static uint64_t last_micros = 0;
    static bool initialized = false;
    // C doesnt allow fn calls in static initializers
    if (!initialized) {
        last_micros = time_us_64();
        initialized = true;
    }
    raw_stick_t raw = {.stick_x_raw = 0.0,
                       .stick_y_raw = 0.0,
                       .stick_x_lin = 0.0,
                       .stick_y_lin = 0.0};

    uint32_t adc_count;
    uint32_t adc_sum_x;
    uint32_t adc_sum_y;

    uint64_t before_micros = time_us_64();
    uint64_t after_micros;
    uint64_t adc_delta;
    do {
        adc_count++;
        adc_sum_x += read_ext_adc(XAXIS);
        adc_sum_y += read_ext_adc(YAXIS);
        after_micros = time_us_64();
        adc_delta = after_micros - before_micros;
        before_micros = after_micros;
    } while ((after_micros - last_micros) < (1000 - adc_delta));

    // Then we spinlock to get the 1 kHz more exactly.
    while (after_micros - last_micros < 1000) {
        after_micros = time_us_64();
    }
    last_micros += 1000;

    raw.stick_x_raw = (float)adc_sum_x / ((float)(ADC_MAX * adc_count));
    raw.stick_y_raw = (float)adc_sum_y / ((float)(ADC_MAX * adc_count));
}

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

    // 3MHz
    spi_init(STICK_SPI_INTF, 3000 * 1000);
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

buttons_t read_buttons() {
    buttons_t btn = {
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
        .c_down = 0,
        .c_up = 0,
        .dpad_right = 0,
        .dpad_left = 0,
        .dpad_down = 0,
        .dpad_up = 0,
    };

    btn.a = !gpio_get(BTN_A_PIN);
    btn.b = !gpio_get(BTN_B_PIN);
    btn.start = !gpio_get(BTN_START_PIN);
    btn.r = !gpio_get(BTN_R_PIN);
    btn.l = !gpio_get(BTN_L_PIN);
#ifdef BTN_ZR_PIN
    btn.zr = !gpio_get(BTN_ZR_PIN);
#endif
    btn.zl = !gpio_get(BTN_ZL_PIN);
    btn.c_right = !gpio_get(BTN_CR_PIN);
    btn.c_left = !gpio_get(BTN_CL_PIN);
    btn.c_up = !gpio_get(BTN_CU_PIN);
    btn.c_down = !gpio_get(BTN_CD_PIN);
    btn.dpad_right = !gpio_get(BTN_DR_PIN);
    btn.dpad_left = !gpio_get(BTN_DL_PIN);
    btn.dpad_down = !gpio_get(BTN_DD_PIN);
    btn.dpad_up = !gpio_get(BTN_DU_PIN);

    return btn;
}