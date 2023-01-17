#ifndef _CONTROL_H
#define _CONTROL_H

#include "Phobri64.h"

#define CMD_GET_STATE         0x00 // GET
#define CMD_START_CALIBRATION 0x01 // SET 
#define CMD_SET_NOTCH_VALUE   0x02 // SET [notch] [value]

#define CALIBRATION_NUM_SAMPLES 128
#define CALIBRATION_NUM_STEPS NUM_NOTCHES * 2

void init_state_machine();

void control_state_machine();

void start_calibration();

uint16_t return_state(uint8_t *buffer, uint16_t bufsize, uint8_t instance);

typedef struct {
    int8_t calibration_step;
    stick_params_t stick_params;
} phobri_state_t;

extern volatile phobri_state_t _state;

#endif /* _CONTROL_H */