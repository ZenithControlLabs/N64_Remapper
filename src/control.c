#include "Phobri64.h"

volatile phobri_state_t _state;

void init_state_machine() {
    _state.calibration_step = 0; // not calibrating

    init_hardware();

    raw_report_t r_report = read_hardware(true);

    // Do any startup checks
        
    // reboot in BOOTSEL mode if start is held
    if (r_report.start) {
        sleep_ms(1);
        reset_usb_boot(0, 0);
    }
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

float raw_cal_points_x[CALIBRATION_NUM_STEPS];
float raw_cal_points_y[CALIBRATION_NUM_STEPS];

void calibration_start() {
    // if for some reason start_calibration is sent after it is already started, just ignore the command
    if (_state.calibration_step < 1) {
        _state.calibration_step = 1;
        printf("Starting calibration!\nCalibration Step [%d/%d]\n", _state.calibration_step, CALIBRATION_NUM_STEPS);
    }
}

void calibration_advance() {
    // failsafe - this function should not be called when incrementing the step would lead to an invalid state
    if (_state.calibration_step < 1 || _state.calibration_step > CALIBRATION_NUM_STEPS) 
        return; 

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

    raw_cal_points_x[_state.calibration_step-1] = x;
    raw_cal_points_y[_state.calibration_step-1] = y;
    printf("Raw X value collected: %f\nRaw Y value collected: %f\n", x, y);
    _state.calibration_step++;

    if (_state.calibration_step > CALIBRATION_NUM_STEPS) {
        calibration_finish();
    } else {
        printf("Calibration Step [%d/%d]\n", _state.calibration_step, CALIBRATION_NUM_STEPS);
    }
    
}

void calibration_undo() {
    // Go back one calibration step, only if we are actually calibrating and not at the beginning.
    if (_state.calibration_step > 1) {
        _state.calibration_step--;
    } 
    printf("Calibration Step [%d/%d]\n", _state.calibration_step, CALIBRATION_NUM_STEPS);
}
bool cringe_global = false;

void calibration_finish() {
    // We're done calibrating. Do the math to save our calibration parameters
    float cleaned_points_x[NUM_NOTCHES + 1];
    float cleaned_points_y[NUM_NOTCHES + 1];
    for (int i = 0; i < CALIBRATION_NUM_STEPS; i++) {
        printf("Raw Cal point:  %d; (x,y) = (%f, %f)\n", i, raw_cal_points_x[i], raw_cal_points_y[i]);
    }
    clean_cal_points(raw_cal_points_x, raw_cal_points_y, cleaned_points_x, cleaned_points_y);
    float linearized_points_x[NUM_NOTCHES + 1];
    float linearized_points_y[NUM_NOTCHES + 1];
    for (int i = 0; i <= NUM_NOTCHES; i++) {
        printf("Clean Cal point:  %d; (x,y) = (%f, %f)\n", i, cleaned_points_x[i], cleaned_points_y[i]);
    }
    linearize_cal(cleaned_points_x, cleaned_points_y, linearized_points_x, linearized_points_y, &(_state.stick_params));
    printf("Calibrated!\n");
    printf("X coeffs: %f %f %f %f, Y coeffs: %f %f %f %f\n", _state.stick_params.fit_coeffs_x[0],
    _state.stick_params.fit_coeffs_x[1],
    _state.stick_params.fit_coeffs_x[2],
    _state.stick_params.fit_coeffs_x[3],
    _state.stick_params.fit_coeffs_y[0],
    _state.stick_params.fit_coeffs_y[1],
    _state.stick_params.fit_coeffs_y[2],
    _state.stick_params.fit_coeffs_y[3]);
    _state.calibration_step = -1;
    cringe_global = true;
}


void control_state_machine() {
    // always read raw hardware report first
    raw_report_t r_report;
    if (_state.calibration_step > 0) {
        // We have nothing to do. Calibration is handled through USB commands.
        // Just communicate default controller state.
        // In the future, if we want to allow controller buttons to advance/undo calibration,
        // you could add button debouncing logic to trigger calibration_advance/calibration_undo here.
        create_default_n64_report();
    } else {
        /*_report = (n64_report_t){
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
        };*/
        r_report = read_hardware(false);
        
        processed_stick_t stick_out;
        process_stick(&r_report, &(_state.stick_params), &stick_out);
        //if (cringe_global) printf("X %d Y %d\n", stick_out.x, stick_out.y);
        //printf("%d %d %d\n", r_report.a, r_report.b,r_report.zl);
        from_raw_report(&r_report, &stick_out);
        
    }
}