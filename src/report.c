#include "Phobri64.h"

volatile n64_report_t _report;

// FIXME why do i have two default n64 reports
void create_default_n64_report(void) {
    _report = (n64_report_t){
        .dpad_right = 0,
        .dpad_left = 0,
        .dpad_down = 0,
        .dpad_up = 0,
        .start = 0,
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
    };
}

void from_raw_report(const raw_report_t *raw_report,
                     processed_stick_t *stick_out) {
    _report = (n64_report_t){.dpad_right = raw_report->dpad_right,
                             .dpad_left = raw_report->dpad_left,
                             .dpad_down = raw_report->dpad_down,
                             .dpad_up = raw_report->dpad_up,
                             .start = raw_report->start,
                             .z = raw_report->zl,
                             .b = raw_report->b,
                             .a = raw_report->a,
                             .c_right = raw_report->c_right,
                             .c_left = raw_report->c_left,
                             .c_up = raw_report->c_up,
                             .c_down = raw_report->c_down,
                             .r = raw_report->r,
                             .l = raw_report->l,
                             .stick_x = stick_out->x,
                             .stick_y = stick_out->y};
}