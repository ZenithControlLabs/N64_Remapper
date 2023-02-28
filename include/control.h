#ifndef _CONTROL_H
#define _CONTROL_H

#include "Phobri64.h"

// Calibration parameters
#define CALIBRATION_NUM_SAMPLES 128
#define CALIBRATION_NUM_STEPS NUM_NOTCHES * 2

// SET report IDs
#define CMD_START_CALIBRATION 0x01 // SET 
#define CMD_INC_CAL_STEP 0x69
#define CMD_DEC_CAL_STEP 0x68
#define CMD_SET_NOTCH_VALUE   0x02 // SET [notch] [value]

// GET report IDs
#define CMD_GET_CAL_STEP     0x00
#define CMD_GET_STICK_PARAMS_DEBUG 0x01
#define CMD_GET_NOTCH_ANGLES 0x02

// Interrupt report IDs

void init_state_machine();

void control_state_machine();

void calibration_start();

void calibration_advance();

void calibration_undo();

void calibration_finish();

uint16_t send_state(uint8_t report_id, uint8_t *buffer, uint16_t bufsize);

typedef struct {
    int8_t calibration_step;
    stick_params_t stick_params;
} phobri_state_t;

extern volatile phobri_state_t _state;

#endif /* _CONTROL_H */