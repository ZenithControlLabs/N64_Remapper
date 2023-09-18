#ifndef JOYBUS_CONS_H
#define JOYBUS_CONS_H

#include <hardware/pio.h>
#include <pico/types.h>
#include <stdint.h>

#include "zenith/comms/n64.h"

#define JOYBUS_CTLR 2

uint joybus_cons_port_init(joybus_port_t *port, uint pin, PIO pio, int sm,
                           int offset);

void init_joybus();

uint32_t read_joybus_ctlr();

#endif // JOYBUS_CONS_H