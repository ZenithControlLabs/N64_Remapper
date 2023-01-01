#include "Phobri64.h"

volatile n64_report_t _report;

void create_default_n64_report(void) {
    _report = (n64_report_t) {
        .dpad_right = 0,
        .dpad_left = 0,
        .dpad_down = 0,
        .dpad_up = 0,
        .start = 1,
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
        .stick_x = 0x7F,
        .stick_y = 0x7F,
    };
}