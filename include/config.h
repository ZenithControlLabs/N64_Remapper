#ifndef _CONFIG_H
#define _CONFIG_H

#include "Phobri64.h"

//////////////////////////
// USB CONFIG COMMANDS //
////////////////////////

// To elucidate how these report IDs are intended to be used,
// the "parameters" that would go into the buffer with the request
// have been written out like a sort of function signature.
// for example, "[x:u32] ->" means it's a SET report that takes in
// "x" which is expected to be unsigned 32-bit (i.e., 4 bytes long)
// "-> [y:u32]" is a GET report that fills the buffer with a 32-bit number, and
// so forth.
//
// However, most of the commands here end up being commands to get/set
// specific settings in the phob. So, we only specify the address of
// the "settings base", which takes up half the report space, and is
// reserved for any "setting", something that has a "getter" and "setter"
// report ID and uses the same parameters for each in the output and input
// respectively. These settings, being shared, live in their own enum.

// clang-format off

static const uint8_t CMD_SETTING_BASE = 0x80;

// SET report IDs
typedef enum {
    CMD_START_CALIBRATION = 0x01,
    CMD_INC_CAL_STEP      = 0x02,
    CMD_DEC_CAL_STEP      = 0x03,
    CMD_COMMIT_SETTINGS   = 0x04,
    CMD_RESET_FACTORY     = 0x05,

    // NO CMDS PAST CMD_SETTING_BASE!
} set_report_cmd_t;

// GET report IDs
typedef enum {
    // -> [cal_step:u32]
    CMD_GET_CAL_STEP = 0x01,

    // NO CMDS PAST CMD_SETTING_BASE!
} get_report_cmd_t;

typedef enum {
    // [xvalue:u8] [yvalue:u8]
    NOTCH_RIGHT            = 0x00,
    NOTCH_UPRIGHT          = 0x01,
    NOTCH_UP               = 0x02,
    NOTCH_UPLEFT           = 0x03,
    NOTCH_LEFT             = 0x04,
    NOTCH_DOWNLEFT         = 0x05,
    NOTCH_DOWN             = 0x06,
    NOTCH_DOWNRIGHT        = 0x07,
    // [enable:bool] 
    DEBUG_REPORTING        = 0x08,
} setting_id_t;

// clang-format on

// used to check if the flash has been written to by us before
// or has other data. spells "phobri64" (little endian)
static const uint64_t magic = 0x34366972626f6870;

typedef struct {
    int8_t calibration_step;
    calib_results_t calib_results;
    stick_config_t stick_config;
    bool report_dbg;
} config_state_t;

extern config_state_t _cfg_st;

void set_setting(setting_id_t st, const uint8_t *buffer);

uint16_t get_setting(setting_id_t st, uint8_t *buffer);

//////////////////
// CALIBRATION //
////////////////

extern mutex_t adc_mtx;

// Calibration parameters
#define CALIBRATION_NUM_SAMPLES 128
#define CALIBRATION_NUM_STEPS NUM_NOTCHES * 2

void calibration_start();

void calibration_advance();

void calibration_undo();

void calibration_finish();

/////////////////////
// STATE HANDLING //
///////////////////

// 256k from start of flash
#define FLASH_OFFSET (256 * 1024)

// Commenting out until needed again
// Non-setting GET reports can probably go through usb.c
// uint16_t send_config_state(uint8_t report_id, uint8_t *buffer,
//                           uint16_t bufsize);

void commit_config_state();

void init_config_state();

#endif /* _COFNIG_H */