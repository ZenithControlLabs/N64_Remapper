#ifndef _JOYBUS_H
#define _JOYBUS_H

#include "joybus.pio.h"
#include <hardware/pio.h>
#include <pico/stdlib.h>

// N64 joybus structs and definitions

enum N64Command {
    PROBE = 0x00,
    RESET = 0xFF,
    POLL = 0x01,
    READ_EXPANSION_BUS = 0x02,
    WRITE_EXPANSION_BUS = 0x03,
};

typedef struct __attribute__((packed)) {
    uint16_t device;
    uint8_t status;
} n64_status_t;

static const n64_status_t default_n64_status = {
    .device = 0x0005,
    .status = 0x02,
};

static const n64_status_t other_status = {
    .device = 0xFFFF,
    .status = 0xFF,
};

/**
 * @brief A structure representing a Joybus instance on a given GPIO pin
 */
typedef struct {
    uint pin;
    PIO pio;
    uint sm;
    uint offset;
    pio_sm_config config;
} joybus_port_t;

/**
 * @brief Interrupt handler for when the RX FIFO is *not* empty.
 *        Disabled when we are transmitting over joybus.
 */
static void rx_fifo_not_empty_handler(void);

/**
 * @brief Interrupt handler for when the RX FIFO is *not* empty.
 *        Always disabled, until we need to transfer something (triggered by
 * RX.)
 */
static void tx_fifo_empty_handler(void);

/**
 * @brief Initialize the joybus PIO program and populate necessary information
 * in a struct
 *
 * @param port Pointer to the port structure to initialize
 * @param pin The pin to use for the joybus instance
 * @param pio The PIO instance; either pio0 or pio1
 * @param sm The state machine to run the joybus instance on. Pass in -1 to
 * automatically claim unused.
 * @param offset The instruction memory offset at which to load the program.
 * Pass in -1 to allocate automatically.
 *
 * @return The offset at which the joybus program is loaded
 */
uint joybus_port_init(joybus_port_t *port, uint pin, PIO pio, int sm,
                      int offset);

/**
 * @brief Initialize the joybus PIO program and enable interrupt handlers
 * necessary for comms.
 */
void joybus_init_comms(void);

#endif /* JOPBUS_H */