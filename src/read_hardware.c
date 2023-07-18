#include "main.h"

joybus_port_t cntlr_port;

void init_hardware() {
    // We are just going to claim PIO1
    joybus_port_init(&cntlr_port, JOYBUS_CTLR, pio1, -1, -1);

    // Send status byte
    uint32_t data_shifted = (PROBE << 24) | (1 << 23);
    pio_sm_put_blocking((&cntlr_port)->pio, (&cntlr_port)->sm, data_shifted);

    int byte_cnt = 0;
    while (byte_cnt < 3) {
        uint32_t d = pio_sm_get_blocking((&cntlr_port)->pio, (&cntlr_port)->sm);
        byte_cnt++;
    }

    joybus_program_send_init((&cntlr_port)->pio, (&cntlr_port)->sm,
                             (&cntlr_port)->offset, (&cntlr_port)->pin,
                             &(&cntlr_port)->config);

    return;
}

n64_report_t read_hardware() {
    uint32_t data_shifted = (POLL << 24) | (1 << 23);
    pio_sm_put_blocking((&cntlr_port)->pio, (&cntlr_port)->sm, data_shifted);

    int byte_cnt = 0;
    uint32_t ctlr_word = 0;
    while (byte_cnt < 4) {
        uint8_t temp_data =
            pio_sm_get_blocking((&cntlr_port)->pio, (&cntlr_port)->sm);
        ctlr_word >>= 8;
        ctlr_word |= (temp_data << 24);
        byte_cnt++;
        // printf("%d ", temp_data);
    }
    // printf("%d\n", ctlr_word);
    // printf("\n");

    joybus_program_send_init((&cntlr_port)->pio, (&cntlr_port)->sm,
                             (&cntlr_port)->offset, (&cntlr_port)->pin,
                             &(&cntlr_port)->config);

    return (*((n64_report_t *)&ctlr_word));
}
