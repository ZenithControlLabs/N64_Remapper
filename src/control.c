#include "Phobri64.h"

volatile phobri_state_t _state;

void init_state_machine() {
    _state.calibration_step = -1; // not calibrating

    init_hardware();
}

void start_calibration() {
    // if for some reason start_calibration is sent after it is already started, just ignore the command
    if (_state.calibration_step < 0) _state.calibration_step = 0;
}

uint16_t return_state(uint8_t *buffer, uint16_t bufsize, uint8_t instance) {
    static int ind = 0;
    static bool consumed = true;
    /*size_t sz = 1 + sizeof(float) * FIT_ORDER * 2;
    if (bufsize < sz) {
        return 0;
    }*/

    // This means we are getting a new request to send data,
    // and we did not finish a previous one. Reset the ind counter.
    if (instance == 0 && !consumed) {
        printf("I reset\n");
        ind = 0;
    }

    int j = 1;
    int buf_ind = 0;
    uint8_t* state_buffer = (uint8_t*)&_state;
    while (ind < sizeof(_state) && buf_ind < bufsize) {
        buffer[buf_ind] = state_buffer[ind];
        ind++;
        buf_ind++;
    }

    if (ind == sizeof(_state)) {
        // we are done so set consumed to true
        printf("Consume %d %d\n", ind, buf_ind);
        consumed = true;
        ind = 0;
    } else { 
        printf("Not Consume %d %d\n", ind, buf_ind);
        consumed = false;
    }

    return buf_ind;
}

void calibration_logic(raw_report_t *report) {
    static float raw_cal_points_x[CALIBRATION_NUM_STEPS];
    static float raw_cal_points_y[CALIBRATION_NUM_STEPS];

    if (_state.calibration_step > CALIBRATION_NUM_STEPS) {
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

        raw_cal_points_x[_state.calibration_step] = x;
        raw_cal_points_y[_state.calibration_step] = y;

        _state.calibration_step++;
    } else if (report->b) {
        // Go back one calibration step.
        if (_state.calibration_step > 0) _state.calibration_step--;
    }
}


void control_state_machine() {
    // always read raw hardware report first
    raw_report_t r_report = read_hardware();
    if (_state.calibration_step >= 0) {
        // we are calibrating
        calibration_logic(&r_report);
    } else {
    }
}