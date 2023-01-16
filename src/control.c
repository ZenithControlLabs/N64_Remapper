#include "Phobri64.h"

volatile phobri_state_t _state;

void init_state_machine() {
    _state.calibration_notch = -1; // not calibrating

    init_hardware();
}

void start_calibration() {
    // if for some reason start_calibration is sent after it is already started, just ignore the command
    if (_state.calibration_notch < 0) _state.calibration_notch = 0;
}

uint16_t return_state(uint8_t *buffer, uint16_t bufsize) {
    size_t sz = 1 + sizeof(float) * FIT_ORDER * 2;
    if (bufsize < sz) {
        return 0;
    }

    buffer[0] = _state.calibration_notch;
    int j = 1;
    for (int i = 0; i < FIT_ORDER; i++) {
        union {
            float f;
            uint8_t b[4];
        } u;
        u.f = _state.stick_params.fit_coeffs_x[i];
        memcpy(buffer+j, u.b, 4);
        j += 4;
        u.f = _state.stick_params.fit_coeffs_y[i];
        memcpy(buffer+j, u.b, 4);
        j += 4;
    }

    return sz;
}

void calibration_logic(raw_report_t *report) {
    static float raw_cal_points_x[CALIBRATION_NUM_POINTS];
    static float raw_cal_points_y[CALIBRATION_NUM_POINTS];

    if (_state.calibration_notch > CALIBRATION_NUM_POINTS) {
        // We're done calibrating. Do the math to save our calibration parameters
        float cleaned_points_x[NUM_NOTCHES + 1];
        float cleaned_points_y[NUM_NOTCHES + 1];
        clean_cal_points(raw_cal_points_x, raw_cal_points_y, cleaned_points_x, cleaned_points_y);
    } else if (report->a) {
        // Capturing the current notch and moving to the next one.
        float x = 0;
        float y = 0;
        // Taking average of readings over CALIBRATION_NUM_SAMPLES number of samples.
        for (int i = 0; i < CALIBRATION_NUM_SAMPLES; i++) {
            x += read_stick_x();
            y += read_stick_y();
        }
        x /= (float)CALIBRATION_NUM_SAMPLES;
        y /= (float)CALIBRATION_NUM_SAMPLES;

        raw_cal_points_x[_state.calibration_notch] = x;
        raw_cal_points_y[_state.calibration_notch] = y;
    } else if (report->b) {
        // Go back one calibration step.
        if (_state.calibration_notch > 0) _state.calibration_notch--;
    }
}


void control_state_machine() {
    // always read raw hardware report first
    raw_report_t r_report = read_hardware();
    if (_state.calibration_notch >= 0) {
        // we are calibrating
        calibration_logic(&r_report);
    } else {
    }
}