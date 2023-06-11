#include "Phobri64.h"

n64_report_t _report;
mutex_t _report_lock;

#ifdef DEBUG
debug_report_t _dbg_report;
#endif

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

void from_raw_report(const raw_report_t *raw_report,
                     processed_stick_t *stick_out) {
    mutex_enter_blocking(&_report_lock);
    _report = (n64_report_t){.dpad_right = raw_report->dpad_right,
                             .dpad_left = raw_report->dpad_left,
                             .dpad_down = raw_report->dpad_down,
                             .dpad_up = raw_report->dpad_up,
                             .start = raw_report->start,
                             .z = raw_report->zl,
                             .b = raw_report->b,
                             .a = raw_report->a,
                             .c_right = raw_report->c_right,
                             .c_left = raw_report->c_left,
                             .c_up = raw_report->c_up,
                             .c_down = raw_report->c_down,
                             .r = raw_report->r,
                             .l = raw_report->l,
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
    // always read raw hardware report first
    raw_report_t r_report = read_hardware(false);
#ifdef DEBUG
    // command to reset the controller without having to replug usb, for
    // debugging purposes
    if (r_report.l & r_report.zl & r_report.r) {
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
#ifdef DEBUG
        _dbg_report.stick_x_raw = r_report.stick_x;
        _dbg_report.stick_y_raw = r_report.stick_y;
#endif
    } else {
        processed_stick_t stick_out;
        process_stick(&r_report, &(_cfg_st.calib_results), &stick_out);
        from_raw_report(&r_report, &stick_out);
    }
}
