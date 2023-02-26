#ifndef PHOBRI64_H_
#define PHOBRI64_H_

// System includes
#include <hardware/pio.h>
#include <hardware/spi.h>
#include <pico/multicore.h>
#include <pico/bootrom.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/util/queue.h>
#include <stdio.h>
#include <math.h>

// Local includes
#include "stick.h" // Where all the stick math lives.
#include "control.h" // The main control state machine running on core1.
#include "joybus.h" // Handle joybus comms. Take in a controller report structure as input.
#include "report.h" // Responsible for updating _report variable, holding the up to date (PROCESSED) controller state.
#include "storage.h" // Responsible for saving stick settings.
#include "usb.h"     // Handle USB communication with host PC.
#include "read_hardware.h" // Interfacing with the hardware on board (ADCs, GPIO)

// Hardware include - uncomment these based on the RP2040 board you are using!
#include "hw/phob2_debug.h"

#endif /* PHOBRI64_H_ */