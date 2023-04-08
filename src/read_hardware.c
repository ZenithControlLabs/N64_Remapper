#include "Phobri64.h"

//for external MCP3202 adc, 12 bit
int __time_critical_func(readExtAdc)(bool isXaxis) {
	//                     start bit
	//                     |absolute, two channels
	//                     ||channel 0
	//                     |||most significant bit first
	//                     ||||(don't care, even though it gets repeated)
	//                     ||||null bit
	//                     |||||11
	//                     ||||||10  byte 1   byte 2 (when read out)
	//                     |||||||9  87654321 0_______
    uint8_t configBits;
    if (isXaxis) {
        configBits = 0b11010000;
    } else {
        configBits = 0b11110000; //channel 1
    }
	uint8_t buf[2];
	gpio_put(STICK_SPI_CS, 0);

	spi_read_blocking(spi0, configBits, buf, 3);
    //debug_print("raw: %x %x %x\n", buf[0], buf[1], buf[2]);
	uint16_t tempValue = (((buf[0] & 0b00000111) << 9) | buf[1] << 1 | buf[2] >> 7);

	gpio_put(STICK_SPI_CS, 1);

	return tempValue;
}

/*
// whichaxis = true => Y
// whichstick = true => analog
int __time_critical_func(readExtAdc)(const bool whichStick, const bool whichAxis) {
	//                        start bit
	//                        |absolute, two channels
	//                        ||channel 0
	//                        |||most significant bit first
	//                        ||||(don't care, even though it gets repeated)
	//                        ||||null bit
	//                        |||||11
	//                        ||||||10  byte 1   byte 2 (when read out)
	//                        |||||||9  87654321 0_______
	uint8_t configBits[] = {0b11010000};
	if(whichAxis) {
		configBits[0] =     0b11110000;//channel 1
	}
	uint8_t buf[3];
	//asm volatile("nop \n nop \n nop");//these were in the example; are they needed?
	if(whichStick) {
		//left stick
		gpio_put(STICK_SPI_CS, 0);
	} else {
		//c-stick
		gpio_put(CSTICK_SPI_CS, 0);
	}
	//asm volatile("nop \n nop \n nop");

	spi_read_blocking(spi0, *configBits, buf, 3);
    debug_print("raw: %x %x %x\n", buf[0], buf[1], buf[2]);
	uint16_t tempValue = (((buf[0] & 0b00000111) << 9) | buf[1] << 1 | buf[2] >> 7);

	//asm volatile("nop \n nop \n nop");
	if(whichStick ) {
		gpio_put(STICK_SPI_CS, 1);
	} else {
		gpio_put(CSTICK_SPI_CS, 1);
	}
	//asm volatile("nop \n nop \n nop");

	return tempValue;
}
*/

inline float read_stick_x() {
    return readExtAdc(true) / 4096.0;
}

inline float read_stick_y() {
    return readExtAdc(false) / 4096.0;
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
    //init_btn_pin(BTN_ZR_PIN);
    init_btn_pin(BTN_ZL_PIN);
    init_btn_pin(BTN_CR_PIN);
    init_btn_pin(BTN_CL_PIN);
    //init_btn_pin(BTN_CD_PIN);
    //init_btn_pin(BTN_CU_PIN);
    init_btn_pin(BTN_DR_PIN);
    init_btn_pin(BTN_DL_PIN);
    init_btn_pin(BTN_DD_PIN);
    init_btn_pin(BTN_DU_PIN);

    // 3MHz
    spi_init(spi0, 3000*1000);
	gpio_set_function(STICK_SPI_CLK, GPIO_FUNC_SPI);
	gpio_set_function(STICK_SPI_TX, GPIO_FUNC_SPI);
	gpio_set_function(STICK_SPI_RX, GPIO_FUNC_SPI);
	gpio_init(STICK_SPI_CS);
	gpio_set_dir(STICK_SPI_CS, GPIO_OUT);
	gpio_put(STICK_SPI_CS, 1); //active low
    #ifdef CSTICK_SPI_CS
    // phob2 devboard has a c-stick SPI interface as well
    // leave chip select high so it doesnt interfere with us
    gpio_init(CSTICK_SPI_CS);
	gpio_set_dir(CSTICK_SPI_CS, GPIO_OUT);
    gpio_put(CSTICK_SPI_CS, 1);
    #endif

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
    //report.zr = !gpio_get(BTN_ZR_PIN);
    report.zl = !gpio_get(BTN_ZL_PIN);
    report.c_right = !gpio_get(BTN_CR_PIN);
    report.c_left = !gpio_get(BTN_CL_PIN);
    report.dpad_right = !gpio_get(BTN_DR_PIN);
    report.dpad_left = !gpio_get(BTN_DL_PIN);
    report.dpad_down = !gpio_get(BTN_DD_PIN);
    report.dpad_up = !gpio_get(BTN_DU_PIN);
    
    return report;
}