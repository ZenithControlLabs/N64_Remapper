#ifndef _READ_HARDWARE_H
#define _READ_HARDWARE_H

#include "Phobri64.h"

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

uint16_t __time_critical_func(readExtAdc)(bool isXaxis)

#define read_stick_x() read_ext_adc(true);
#define read_stick_y() read_ext_adc(false);

void init_hardware();

raw_report_t read_hardware(bool quick);

#endif /* _READ_HARDWARE_H */