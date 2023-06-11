#ifndef _CONFIG_H
#define _CONFIG_H

#include "Phobri64.h"

// Calibration parameters
#define CALIBRATION_NUM_SAMPLES 128
#define CALIBRATION_NUM_STEPS NUM_NOTCHES * 2

// SET report IDs
#define CMD_START_CALIBRATION 0x01 // SET
#define CMD_INC_CAL_STEP 0x69
#define CMD_DEC_CAL_STEP 0x68
#define CMD_SET_NOTCH_VALUE 0x02 // SET [notch] [value]
#define CMD_COMMIT_SETTINGS 0x03

// GET report IDs
#define CMD_GET_CAL_STEP 0x00
#define CMD_GET_calib_results_DEBUG 0x01
#define CMD_GET_NOTCH_ANGLES 0x02

// Interrupt report IDs
// N/A

typedef struct {
    int8_t calibration_step;
    calib_results_t calib_results;
    stick_config_t stick_config;
} config_state_t;

extern config_state_t _cfg_st;

//////////////////
// CALIBRATION //
////////////////

void calibration_start();

void calibration_advance();

void calibration_undo();

void calibration_finish();

/////////////////////
// STATE HANDLING //
///////////////////

uint16_t send_config_state(uint8_t report_id, uint8_t *buffer,
                           uint16_t bufsize);

void commit_config_state();

void init_config_state();

#endif /* _COFNIG_H */