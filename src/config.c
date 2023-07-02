#include "Phobri64.h"

config_state_t _cfg_st;

///////////////
// SETTINGS //
/////////////

void set_setting(setting_id_t st, const uint8_t *buffer) {
    // notch settings go from 0 (right) counterclockwise to 7 downright
    if (st <= NOTCH_DOWNRIGHT) {
        debug_print("%d %d %d\n", st, buffer[0], buffer[1]);
        _cfg_st.stick_config.notch_points_x[st] = buffer[0];
        _cfg_st.stick_config.notch_points_y[st] = buffer[1];
        // recompute notch calibration
        notch_calibrate(_cfg_st.stick_config.linearized_points_x,
                        _cfg_st.stick_config.linearized_points_y,
                        _cfg_st.stick_config.notch_points_x,
                        _cfg_st.stick_config.notch_points_y,
                        &(_cfg_st.calib_results));
    }
    switch (st) {
    case DEBUG_REPORTING:
        _cfg_st.report_dbg = buffer[0];
        break;
    }
}

uint16_t get_setting(setting_id_t st, uint8_t *buffer) {
    // no settings implemented here at the moment (#4)
    uint16_t sz = 0;
    return sz;
}

//////////////////
// CALIBRATION //
////////////////

float raw_cal_points_x[CALIBRATION_NUM_STEPS];
float raw_cal_points_y[CALIBRATION_NUM_STEPS];

mutex_t adc_mtx;

void calibration_start() {
    // if for some reason start_calibration is sent after it is already started,
    // just ignore the command
    if (_cfg_st.calibration_step < 1) {
        _cfg_st.calibration_step = 1;
        debug_print("Starting calibration!\nCalibration Step [%d/%d]\n",
                    _cfg_st.calibration_step, CALIBRATION_NUM_STEPS);
    }
}

void calibration_advance() {
    // failsafe - this function should not be called if incrementing the step
    // would lead to an invalid state
    if (_cfg_st.calibration_step < 1 ||
        _cfg_st.calibration_step > CALIBRATION_NUM_STEPS)
        return;

    // Capturing the current notch and moving to the next one.
    uint32_t x = 0;
    uint32_t y = 0;
    // Taking average of readings over CALIBRATION_NUM_SAMPLES number of
    // samples.
    // Hold the ADC mutex in this time
    mutex_enter_blocking(&adc_mtx);
    for (int i = 0; i < CALIBRATION_NUM_SAMPLES; i++) {
        x += read_ext_adc(XAXIS);
        y += read_ext_adc(YAXIS);
    }
    mutex_exit(&adc_mtx);
    float xf = (float)x / ((float)(ADC_MAX * CALIBRATION_NUM_SAMPLES));
    float yf = (float)y / ((float)(ADC_MAX * CALIBRATION_NUM_SAMPLES));

    raw_cal_points_x[_cfg_st.calibration_step - 1] = xf;
    raw_cal_points_y[_cfg_st.calibration_step - 1] = yf;
    debug_print("Raw X value collected: %f\nRaw Y value collected: %f\n", xf,
                yf);
    _cfg_st.calibration_step++;

    if (_cfg_st.calibration_step > CALIBRATION_NUM_STEPS) {
        calibration_finish();
    } else {
        debug_print("Calibration Step [%d/%d]\n", _cfg_st.calibration_step,
                    CALIBRATION_NUM_STEPS);
    }
}

void calibration_undo() {
    // Go back one calibration step, only if we are actually calibrating and not
    // at the beginning.
    if (_cfg_st.calibration_step > 1) {
        _cfg_st.calibration_step--;
    }
    debug_print("Calibration Step [%d/%d]\n", _cfg_st.calibration_step,
                CALIBRATION_NUM_STEPS);
}

void calibration_finish() {
    // TODO comment all of this and make it decent
    // We're done calibrating. Do the math to save our calibration parameters
    float cleaned_points_x[NUM_NOTCHES + 1];
    float cleaned_points_y[NUM_NOTCHES + 1];
    for (int i = 0; i < CALIBRATION_NUM_STEPS; i++) {
        debug_print("Raw Cal point:  %d; (x,y) = (%f, %f)\n", i,
                    raw_cal_points_x[i], raw_cal_points_y[i]);
    }
    fold_center_points(raw_cal_points_x, raw_cal_points_y, cleaned_points_x,
                       cleaned_points_y);
    // One less because center is becoming 0 implcitly
    float linearized_points_x[NUM_NOTCHES];
    float linearized_points_y[NUM_NOTCHES];
    for (int i = 0; i <= NUM_NOTCHES; i++) {
        debug_print("Clean Cal point:  %d; (x,y) = (%f, %f)\n", i,
                    cleaned_points_x[i], cleaned_points_y[i]);
    }
    linearize_cal(cleaned_points_x, cleaned_points_y, linearized_points_x,
                  linearized_points_y, &(_cfg_st.calib_results));
    // Copy the linearized points we have just found to phobri's internal data
    // sturcture.
    for (int i = 0; i < NUM_NOTCHES; i++) {
        _cfg_st.stick_config.linearized_points_x[i] = linearized_points_x[i];
        _cfg_st.stick_config.linearized_points_y[i] = linearized_points_y[i];
        debug_print("Linearized point:  %d; (x,y) = (%f, %f)\n", i,
                    linearized_points_x[i], linearized_points_y[i]);
    }

    notch_calibrate(linearized_points_x, linearized_points_y,
                    _cfg_st.stick_config.notch_points_x,
                    _cfg_st.stick_config.notch_points_y,
                    &(_cfg_st.calib_results));
    debug_print("Calibrated!\n");
    debug_print("X coeffs: %f %f %f %f, Y coeffs: %f %f %f %f\n",
                _cfg_st.calib_results.fit_coeffs_x[0],
                _cfg_st.calib_results.fit_coeffs_x[1],
                _cfg_st.calib_results.fit_coeffs_x[2],
                _cfg_st.calib_results.fit_coeffs_x[3],
                _cfg_st.calib_results.fit_coeffs_y[0],
                _cfg_st.calib_results.fit_coeffs_y[1],
                _cfg_st.calib_results.fit_coeffs_y[2],
                _cfg_st.calib_results.fit_coeffs_y[3]);
    _cfg_st.calibration_step = -1;
}

/////////////////////
// STATE HANDLING //
///////////////////

uint16_t send_config_state(uint8_t report_id, uint8_t *buffer,
                           uint16_t bufsize) {}

// 256k from start of flash
#define FLASH_OFFSET (256 * 1024)
void __not_in_flash_func(commit_config_state)() {
    uint8_t settings_buf[FLASH_SECTOR_SIZE];

    memcpy(settings_buf, (uint8_t *)(&_cfg_st), sizeof(_cfg_st));

    debug_print("Start lockout..\n");
    multicore_lockout_start_blocking();
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_OFFSET, settings_buf, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
    multicore_lockout_end_blocking();
    debug_print("Successfully wrote to flash!..\n");
}

// This is a short method but it could be expanded in the future as we add
// factory defaults for certain settings.
void init_config_state() {
    // load our configuration from flash
    memcpy((uint8_t *)(&_cfg_st), (void *)(XIP_BASE + FLASH_OFFSET),
           sizeof(config_state_t));

    // for now, until we add a method to load stuff from factory
    for (int i = 0; i < NUM_NOTCHES; i++) {
        _cfg_st.stick_config.notch_points_x[i] = perfect_notches_x[i];
        _cfg_st.stick_config.notch_points_y[i] = perfect_notches_y[i];
    }

    mutex_init(&adc_mtx);

    _cfg_st.calibration_step = 0; // not calibrating
}