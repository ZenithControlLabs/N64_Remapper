#ifndef PHOBRI64_H_
#define PHOBRI64_H_

// System includes
#include <hardware/pio.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <pico/time.h>
#include <pico/util/queue.h>
#include <stdio.h>

// Local includes
#include "joybus.h" // Handle joybus comms. Take in a controller report structure as input.
#include "params.h" // Manage current stick parameters.
#include "report.h" // Responsible for updating _report variable, holding the up to date (PROCESSED) controller state.
#include "storage.h" // Responsible for saving stick settings.
#include "usb.h"     // Handle USB communication with host PC.

// Hardware include - uncomment these based on the RP2040 board you are using!
#include "hw/pico_debug.h"

#endif /* PHOBRI64_H_ */