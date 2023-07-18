#include "main.h"

// The raw stick is global so it can be reported thru USB
// as a debugging feature.
raw_stick_t _raw;

n64_report_t _report;
mutex_t _report_lock;

void create_default_n64_report(void) {
    _report = (n64_report_t){
        .dpad_right = 0,
        .dpad_left = 0,
        .dpad_down = 0,
        .dpad_up = 0,
        .start = 0,
        .z = 0,
        .b = 0,
        .a = 0,
        .c_right = 0,
        .c_left = 0,
        .c_down = 0,
        .c_up = 0,
        .r = 0,
        .l = 0,
        .reserved1 = 0,
        .reserved0 = 0,
        .stick_x = 0x0,
        .stick_y = 0x0,
    };
}

// If this function sounds incredibly generic, it's because it is. It's the main
// loop body in our core1.
//
// We read the raw hardware, and process any special commands, then process the
// stick data using our calibration steps.
void process_controller() {

    if (calibration_step > 0) {
        // We have nothing to do. Calibration is handled through USB commands.
        // Just communicate default controller state.
        // In the future, if we want to allow controller buttons to advance/undo
        // calibration, you could add button debouncing logic to trigger
        // calibration_advance/calibration_undo here.
        create_default_n64_report();
    } else {
        mutex_enter_blocking(&adc_mtx);
        n64_report_t raw_report = read_hardware();
        _raw.stick_x_raw = ((float)raw_report.stick_x + 128.0) / 256.0;
        _raw.stick_y_raw = ((float)raw_report.stick_y + 128.0) / 256.0;
        mutex_exit(&adc_mtx);
        processed_stick_t stick_out;
        process_stick(&_raw, &(_cfg_st.calib_results), &stick_out);
        raw_report.stick_x = stick_out.x;
        raw_report.stick_y = stick_out.y;
        mutex_enter_blocking(&_report_lock);
        memcpy(&_report, &raw_report, sizeof(n64_report_t));
        mutex_exit(&_report_lock);
    }
}
