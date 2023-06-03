#include "Phobri64.h"

volatile phobri_state_t _state;

//////////////////
// CALIBRATION //
////////////////

float raw_cal_points_x[CALIBRATION_NUM_STEPS];
float raw_cal_points_y[CALIBRATION_NUM_STEPS];

void calibration_start() {
    // if for some reason start_calibration is sent after it is already started,
    // just ignore the command
    if (_state.calibration_step < 1) {
        _state.calibration_step = 1;
        debug_print("Starting calibration!\nCalibration Step [%d/%d]\n",
                    _state.calibration_step, CALIBRATION_NUM_STEPS);
    }
}

void calibration_advance() {
    // failsafe - this function should not be called when incrementing the step
    // would lead to an invalid state
    if (_state.calibration_step < 1 ||
        _state.calibration_step > CALIBRATION_NUM_STEPS)
        return;

    // Capturing the current notch and moving to the next one.
    float x = 0;
    float y = 0;
    // Taking average of readings over CALIBRATION_NUM_SAMPLES number of
    // samples.
    for (int i = 0; i < CALIBRATION_NUM_SAMPLES; i++) {
        x += read_stick_x();
        y += read_stick_y();
    }
    x /= (float)CALIBRATION_NUM_SAMPLES;
    y /= (float)CALIBRATION_NUM_SAMPLES;

    raw_cal_points_x[_state.calibration_step - 1] = x;
    raw_cal_points_y[_state.calibration_step - 1] = y;
    debug_print("Raw X value collected: %f\nRaw Y value collected: %f\n", x, y);
    _state.calibration_step++;

    if (_state.calibration_step > CALIBRATION_NUM_STEPS) {
        calibration_finish();
    } else {
        debug_print("Calibration Step [%d/%d]\n", _state.calibration_step,
                    CALIBRATION_NUM_STEPS);
    }
}

void calibration_undo() {
    // Go back one calibration step, only if we are actually calibrating and not
    // at the beginning.
    if (_state.calibration_step > 1) {
        _state.calibration_step--;
    }
    debug_print("Calibration Step [%d/%d]\n", _state.calibration_step,
                CALIBRATION_NUM_STEPS);
}

void calibration_finish() {
    // We're done calibrating. Do the math to save our calibration parameters
    float cleaned_points_x[NUM_NOTCHES + 1];
    float cleaned_points_y[NUM_NOTCHES + 1];
    for (int i = 0; i < CALIBRATION_NUM_STEPS; i++) {
        debug_print("Raw Cal point:  %d; (x,y) = (%f, %f)\n", i,
                    raw_cal_points_x[i], raw_cal_points_y[i]);
    }
    clean_cal_points(raw_cal_points_x, raw_cal_points_y, cleaned_points_x,
                     cleaned_points_y);
    float linearized_points_x[NUM_NOTCHES + 1];
    float linearized_points_y[NUM_NOTCHES + 1];
    for (int i = 0; i <= NUM_NOTCHES; i++) {
        debug_print("Clean Cal point:  %d; (x,y) = (%f, %f)\n", i,
                    cleaned_points_x[i], cleaned_points_y[i]);
    }
    linearize_cal(cleaned_points_x, cleaned_points_y, linearized_points_x,
                  linearized_points_y, &(_state.calib_results));

    float perfect_notches_x[] = {0, 100, 0, -100, 0, 75, -75, -75, 75};
    float perfect_notches_y[] = {0, 0, 100, 0, -100, 75, 75, -75, -75};
    notch_calibrate(linearized_points_x, linearized_points_y, perfect_notches_x,
                    perfect_notches_y, &(_state.calib_results));
    debug_print("Calibrated!\n");
    debug_print("X coeffs: %f %f %f %f, Y coeffs: %f %f %f %f\n",
                _state.calib_results.fit_coeffs_x[0],
                _state.calib_results.fit_coeffs_x[1],
                _state.calib_results.fit_coeffs_x[2],
                _state.calib_results.fit_coeffs_x[3],
                _state.calib_results.fit_coeffs_y[0],
                _state.calib_results.fit_coeffs_y[1],
                _state.calib_results.fit_coeffs_y[2],
                _state.calib_results.fit_coeffs_y[3]);
    _state.calibration_step = -1;
}

//////////////////////
// STATE SER/DESER //
////////////////////

uint16_t send_state(uint8_t report_id, uint8_t *buffer, uint16_t bufsize) {
    uint16_t sz = 0;
    switch (report_id) {
    case CMD_GET_CAL_STEP: {
        buffer[0] = _state.calibration_step;
        sz = 1;
        debug_print("Get cal step..\n");
        break;
    }
    default:
        sz = 0;
        break;
    }

    return sz;
}

// 256k from start of flash
#define FLASH_OFFSET (256 * 1024)
void __not_in_flash_func(commit_state)() {
    uint8_t settings_buf[FLASH_SECTOR_SIZE];

    memcpy(settings_buf, (uint8_t *)(&_state), sizeof(_state));

    multicore_lockout_start_blocking();
    uint32_t ints = save_and_disable_interrupts();
    // flash_range_erase(FLASH_OFFSET, FLASH_SECTOR_SIZE);
    // flash_range_program(FLASH_OFFSET, settings_buf, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
    multicore_lockout_end_blocking();
    debug_print("Wrote settings to flash.\n");
}

void load_state() {
    /*memcpy((uint8_t *)(&_state), (void *)(XIP_BASE + FLASH_OFFSET),
           sizeof(phobri_state_t));
    */

    const float fit_coeffs_x_phob[] = {-2180.96582031, 3341.85083008,
                                       -1943.22387695, 420.425537109};
    const float fit_coeffs_y_phob[] = {-1494.0579834, 2152.84741211,
                                       -1292.37756348, 309.844512939};

    const float affine_coeffs_phob[][4] = {
        {
            1.002412,
            0.086960,
            0.001257,
            1.180123,
        },
        {
            0.985584,
            0.071434,
            0.024828,
            1.033578,
        },

        {
            1.197634,
            -0.033903,
            0.018702,
            1.003961,
        },

        {
            1.015719,
            0.021100,
            -0.056041,
            0.917316,
        },

        {
            1.001145,
            -0.102072,
            0.049480,
            1.172863,
        },

        {
            0.833464,
            0.088183,
            -0.053831,
            1.015975,
        },

        {
            1.140362,
            -0.047731,
            0.119106,
            1.008415,
        },

        {
            1.022085,
            -0.010969,
            -0.136205,
            0.791678,
        },
    };

    const float boundary_angles_phob[] = {
        -0.001065, 0.747339, 1.542496, 2.344993,
        3.099430,  3.904683, 4.670557, 5.434765,
    };
    for (int i = 0; i < 4; i++) {
        _state.calib_results.fit_coeffs_x[i] = fit_coeffs_x_phob[i];
        _state.calib_results.fit_coeffs_y[i] = fit_coeffs_y_phob[i];
    }
    for (int i = 0; i < NUM_NOTCHES; i++) {
        _state.calib_results.boundary_angles[i] = boundary_angles_phob[i];
        for (int j = 0; j < 4; j++) {
            _state.calib_results.affine_coeffs[i][j] = affine_coeffs_phob[i][j];
        }
    }
}

///////////////////////////////////
// MAIN STATE MACHINE FUNCTIONS //
/////////////////////////////////

void init_state_machine() {
    // load our configuration from flash
    load_state();
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

void control_state_machine() {
    // always read raw hardware report first
    raw_report_t r_report = read_hardware(false);
#ifdef DEBUG
    // command to reset the controller without having to replug usb, for
    // debugging purposes
    if (r_report.l & r_report.zl & r_report.r) {
        reset_usb_boot(0, 0);
    }
#endif
    if (_state.calibration_step > 0) {
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
        r_report = read_hardware(false);

        processed_stick_t stick_out;
        process_stick(&r_report, &(_state.calib_results), &stick_out);
        from_raw_report(&r_report, &stick_out);
    }
}