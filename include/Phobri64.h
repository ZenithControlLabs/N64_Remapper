#ifndef PHOBRI64_H_
#define PHOBRI64_H_

// System includes
#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>
#include <hardware/pio.h>

// Local includes
#include "joybus.h"  // Handle joybus comms. Take in a controller report structure as input.
#include "report.h"  // Responsible for updating _report variable, holding the up to date (PROCESSED) controller state.
#include "params.h"  // Manage current stick parameters.
#include "usb.h"     // Handle USB communication with host PC.
#include "storage.h" // Responsible for saving stick settings.

// Hardware include - uncomment these based on the RP2040 board you are using!
#include "hw/pico_debug.h"

#endif /* PHOBRI64_H_ */