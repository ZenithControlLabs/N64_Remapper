#ifndef _READ_HARDWARE_H
#define _READ_HARDWARE_H

#include "Phobri64.h"

#define ADC_MAX 4096

typedef struct __attribute__((packed)) {    
    bool a : 1;
    bool b : 1;
    bool start : 1;
    bool r : 1;
    bool l : 1;
    bool zr : 1;
    bool zl : 1;
    bool reserved0 : 1;

    bool c_right : 1;
    bool c_left : 1;
    bool c_down : 1;
    bool c_up : 1;
    bool dpad_right : 1;
    bool dpad_left : 1;
    bool dpad_down : 1;
    bool dpad_up : 1;

    // Units of the following two fields are in voltage (as read by ADC).
    float stick_x;
    float stick_y;
} raw_report_t;

float read_stick_x();

float read_stick_y();

void init_hardware();

raw_report_t read_hardware();

#endif /* _READ_HARDWARE_H */