#include "Phobri64.h"
#include "bsp/board.h"
#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) { return; }

// Invoked when device is unmounted
void tud_umount_cb(void) { return; }

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    return;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) { return; }

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report,
                                uint16_t len) {
    (void)instance;
    (void)report;
    (void)len;

    return;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t bufsize) {
    // TODO not Implemented
    (void)report_type;

    return send_config_state(report_id, buffer, bufsize);
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    // This example doesn't use multiple report and report ID
    (void)instance;
    (void)report_type;
    (void)bufsize;

    switch (report_id) {
    case CMD_START_CALIBRATION:
        calibration_start();
        break;
    case CMD_INC_CAL_STEP:
        calibration_advance();
        break;
    case CMD_DEC_CAL_STEP:
        calibration_undo();
        break;
    case CMD_COMMIT_SETTINGS:
        _pleaseCommit = true;
        break;
    case CMD_SET_NOTCH_VALUE:
        return;
    }

    return;
}

hid_gamepad_report_t convertN64toHIDReport() {
    // Joybus is not allowed to interrupt us here.
    // We are reading the report, and thus claiming the critical section
    uint32_t ints = save_and_disable_interrupts();
    mutex_enter_blocking(&_report_lock);

    uint32_t buttons = (_report.a) << 0 | (_report.b) << 1 | (_report.z) << 2 |
                       (_report.start) << 3 | (_report.l) << 4 |
                       (_report.r) << 5 | (_report.c_up) << 6 |
                       (_report.c_down) << 7 | (_report.c_left) << 8 |
                       (_report.c_right) << 9 | (_report.dpad_up) << 10 |
                       (_report.dpad_down) << 11 | (_report.dpad_left) << 12 |
                       (_report.dpad_right) << 13;

    // Y is multiplied by -1 because HID input seems to flip the axes.
    hid_gamepad_report_t report = {.x = _report.stick_x,
                                   .y = -_report.stick_y,
                                   .z = 0,
                                   .rz = 0,
                                   .rx = 0,
                                   .ry = 0,
                                   .hat = GAMEPAD_HAT_CENTERED,
                                   .buttons = buttons};
    mutex_exit(&_report_lock);
    restore_interrupts(ints);
    return report;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report() {
    static bool which = false;
    // skip if hid is not ready yet
    if (!tud_hid_ready())
        return;

    hid_gamepad_report_t report = convertN64toHIDReport();
#ifdef DEBUG
    // we don't care that much about locking for debug reports.
    if (which)
        tud_hid_report(0x5, &_dbg_report, sizeof(report));
    else
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
    which = !which;
#else
    tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
#endif
}

uint16_t send_custom_report(uint8_t cmd) {
    // if hid is not ready yet let the consumer know
    if (!tud_hid_ready())
        return -1;

    tud_hid_report(cmd, NULL, 0);
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc
// ..) tud_hid_report_complete_cb() is used to send the next report after
// previous one is complete
void hid_task(void) {
// Poll every 10ms
#ifdef DEBUG
    const uint32_t interval_ms = 5;
#else
    const uint32_t interval_ms = 10;
#endif
    static uint32_t start_ms = 0;

    if (to_ms_since_boot(get_absolute_time()) - start_ms < interval_ms)
        return; // not enough time
    start_ms += interval_ms;

    send_hid_report();
}

void usb_init_comms() { tusb_init(); }

void usb_run_comms() {
    tud_task();
    hid_task();
}