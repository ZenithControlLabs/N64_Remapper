#include "Phobri64.h"

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

void update_n64_report(const buttons_t *btn, processed_stick_t *stick_out) {
    mutex_enter_blocking(&_report_lock);
    _report = (n64_report_t){.dpad_right = btn->dpad_right,
                             .dpad_left = btn->dpad_left,
                             .dpad_down = btn->dpad_down,
                             .dpad_up = btn->dpad_up,
                             .start = btn->start,
                             .z = btn->zl,
                             .b = btn->b,
                             .a = btn->a,
                             .c_right = btn->c_right,
                             .c_left = btn->c_left,
                             .c_up = btn->c_up,
                             .c_down = btn->c_down,
                             .r = btn->r,
                             .l = btn->l,
                             .stick_x = stick_out->x,
                             .stick_y = stick_out->y};
    mutex_exit(&_report_lock);
}

// If this function sounds incredibly generic, it's because it is. It's the main
// loop body in our core1.
//
// We read the raw hardware, and process any special commands, then process the
// stick data using our calibration steps.
void process_controller() {

    // always start out the loop doing our multisample ADC read.
    // the only situation that could cause problems is when we're
    // calibrating and the other core is trying to read the ADC.
    // we don't want them to both read, so we wrap them in a critical section.
    mutex_enter_blocking(&adc_mtx);
    _raw = read_stick_multisample();
    mutex_exit(&adc_mtx);

    buttons_t btn = read_buttons();

#ifdef DEBUG
    // command to reset the controller without having to replug usb, for
    // debugging purposes
    if (btn.l & btn.zl & btn.r) {
        reset_usb_boot(0, 0);
    }
#endif

    if (_cfg_st.calibration_step > 0) {
        // We have nothing to do. Calibration is handled through USB commands.
        // Just communicate default controller state.
        // In the future, if we want to allow controller buttons to advance/undo
        // calibration, you could add button debouncing logic to trigger
        // calibration_advance/calibration_undo here.
        create_default_n64_report();
    } else {
        processed_stick_t stick_out;
        process_stick(&_raw, &(_cfg_st.calib_results), &stick_out);
        update_n64_report(&btn, &stick_out);
    }
}
