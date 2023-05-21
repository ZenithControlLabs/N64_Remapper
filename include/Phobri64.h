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
#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/util/queue.h>

/////////////////////
// Local includes //
///////////////////
#include "control.h" // The main control state machine running on core1.
#include "joybus.h" // Handle joybus comms. Take in a controller report structure as input.
#include "read_hardware.h" // Interfacing with the hardware on board (ADCs, GPIO)
#include "report.h" // Responsible for updating _report variable, holding the up to date (PROCESSED) controller state.
#include "stick.h"  // Where all the stick math lives.
#include "usb.h"    // Handle USB communication with host PC.

// This was planned but to keep things simple for now the
// flash storage fns are called in control.c in the commit_settings function.
//#include "storage.h" // Responsible for saving stick settings.

// Hardware include - uncomment these based on the RP2040 board you are using!
#include "hw/phob2_debug.h"

// Enable or disable printf based on whether or not we're making a debug build.
#ifdef DEBUG
#define debug_print(fmt, args...) printf(fmt, ##args)

// TODO: maybe put flags here for various parts of the system?
#else
#define debug_print(fmt, args...)
#endif

#endif /* PHOBRI64_H_ */