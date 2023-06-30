#ifndef REPORT_H_
#define REPORT_H_

#include "Phobri64.h"

typedef struct __attribute__((packed)) {
    bool dpad_right : 1;
    bool dpad_left : 1;
    bool dpad_down : 1;
    bool dpad_up : 1;
    bool start : 1;
    bool z : 1;
    bool b : 1;
    bool a : 1;

    bool c_right : 1;
    bool c_left : 1;
    bool c_down : 1;
    bool c_up : 1;
    bool r : 1;
    bool l : 1;
    uint8_t reserved1 : 1;
    uint8_t reserved0 : 1;

    int8_t stick_x;
    int8_t stick_y;
} n64_report_t;

extern n64_report_t _report;
extern mutex_t _report_lock;

// Debug reporting features
#ifdef DEBUG // <<< REMOVE THIS SHIT

typedef struct {
    float stick_x_raw;
    float stick_y_raw;
    float stick_x_lin;
    float stick_y_lin;
} debug_report_t; // TODO rename this shit to raw report,
// then we can report it if debugging reporting is enabled

extern debug_report_t _dbg_report;

#endif

void create_default_n64_report(void);

void update_n64_report(const buttons_t *btn, processed_stick_t *stick_out);

void process_controller();

#endif /* REPORT_H_ */