#ifndef _READ_HARDWARE_H
#define _READ_HARDWARE_H

#include "Phobri64.h"

const uint16_t ADC_MAX = 4096;

typedef enum { XAXIS, YAXIS } axis_t;

typedef struct __attribute__((packed)) {
    bool a : 1;
    bool b : 1;
    bool start : 1;
    bool r : 1;
    bool l : 1;
    bool zr : 1;
    bool zl : 1;
    bool reserved0 : 1; // padding

    bool c_right : 1;
    bool c_left : 1;
    bool c_down : 1;
    bool c_up : 1;
    bool dpad_right : 1;
    bool dpad_left : 1;
    bool dpad_down : 1;
    bool dpad_up : 1;
} buttons_t;

uint16_t read_ext_adc(axis_t which_axis);

void init_hardware();

buttons_t read_buttons();

#endif /* _READ_HARDWARE_H */