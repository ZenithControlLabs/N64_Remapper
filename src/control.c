#include "Phobri64.h"

volatile phobri_state_t _state;

void init_state_machine() {
    _state.calibration_step = -1; // not calibrating

    init_hardware();
}


uint16_t send_state(uint8_t report_id, uint8_t *buffer, uint16_t bufsize) {

    uint16_t sz = 0;
    switch (report_id) {
        case CMD_GET_CAL_STEP: {
            buffer[0] = _state.calibration_step;
            sz = 1;
            printf("Get cal step..\n");
            break;
        }
        default: sz = 0; break;
    }
    
    return sz;
}

void start_calibration() {
    // if for some reason start_calibration is sent after it is already started, just ignore the command
    if (_state.calibration_step < 0) {
        _state.calibration_step = 0;
        printf("Starting calibration!\nCalibration Step [%d/%d]\n", _state.calibration_step, CALIBRATION_NUM_STEPS);
    }
}

void calibration_logic(raw_report_t *report) {
    static float raw_cal_points_x[CALIBRATION_NUM_STEPS];
    static float raw_cal_points_y[CALIBRATION_NUM_STEPS];

    if (_state.calibration_step > CALIBRATION_NUM_STEPS) {
        // We're done calibrating. Do the math to save our calibration parameters
        float cleaned_points_x[NUM_NOTCHES + 1];
        float cleaned_points_y[NUM_NOTCHES + 1];
        clean_cal_points(raw_cal_points_x, raw_cal_points_y, cleaned_points_x, cleaned_points_y);
        float linearized_points_x[NUM_NOTCHES + 1];
        float linearized_points_y[NUM_NOTCHES + 1];
        linearize_cal(cleaned_points_x, cleaned_points_y, linearized_points_x, linearized_points_y, &(_state.stick_params));
        printf("Calibrated!\n");
        _state.calibration_step = -1;
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

        send_custom_report(CMD_INC_CAL_STEP);
        _state.calibration_step++;

        printf("Calibration Step [%d/%d]\n", _state.calibration_step, CALIBRATION_NUM_STEPS);
    } else if (report->b) {
        // Go back one calibration step.
        if (_state.calibration_step > 0) {
            send_custom_report(CMD_DEC_CAL_STEP);
            _state.calibration_step--;
        } 
        printf("Calibration Step [%d/%d]\n", _state.calibration_step, CALIBRATION_NUM_STEPS);
    }
}


void control_state_machine() {
    // always read raw hardware report first
    raw_report_t r_report = read_hardware();
    if (_state.calibration_step >= 0) {
        // we are calibrating
        calibration_logic(&r_report);
        // Communicate default controller state.
        create_default_n64_report();
    } else {
        _report = (n64_report_t){
            .dpad_right = 0,
            .dpad_left = 0,
            .dpad_down = 0,
            .dpad_up = 0,
            .start = 1,
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
}