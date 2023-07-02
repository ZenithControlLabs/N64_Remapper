#ifndef PHOBRI64_H_
#define PHOBRI64_H_

//////////////////////
// System includes //
////////////////////

// sys
#include <math.h>
#include <stdio.h>

// hw
#include <hardware/flash.h>
#include <hardware/pio.h>
#include <hardware/spi.h>

// pico
#include <pico/bootrom.h>
#include <pico/lock_core.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/util/queue.h>

/////////////////////
// Local includes //
///////////////////
// this one needs to go first to avoid compile errors
#include "stick.h" // Where all the stick math lives.
// and this second
#include "read_hardware.h" // Interfacing with the hardware on board (ADCs, GPIO)

#include "config.h" // Everything related to the config of the stick. Calibration procedure, settings, etc.
#include "joybus.h" // Handle joybus comms. Take in a controller report structure as input.
#include "report.h" // Responsible for reporting out the hardware, from stick to N64 report data structure.
#include "usb.h"    // Handle USB communication with host PC.

// This was planned but to keep things simple for now the
// flash storage fns are called in control.c in the commit_settings function.
// #include "storage.h" // Responsible for saving stick settings.

// Hardware include - uncomment these based on the RP2040 board you are using!
// #include "hw/phob2_debug.h"
#include "hw/phobri_proto.h"

// Core 1 doesn't like to be interrupted with multicore_lockout_blocking for
// some reason, even when it calls multicore_lockout_victim_init(). So, when we
// need to stop the other core and commit the settings, we will just set a flag
// to tell core 1 to do it.
extern bool _please_commit;

// Enable or disable printf based on whether or not we're making a debug build.
#ifdef DEBUG
#define debug_print(fmt, args...) printf(fmt, ##args)

// TODO: maybe put flags here for various parts of the system?
#else
#define debug_print(fmt, args...)
#endif

#endif /* PHOBRI64_H_ */