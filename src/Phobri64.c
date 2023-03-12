#include "Phobri64.h"

void second_core() {
  init_state_machine();
  create_default_n64_report();
  stdio_uart_init_full(uart0, 115200, DEBUG_TX_PIN, -1);

  printf("Phobri64 Initialization\n");
  while (true) {
    control_state_machine();
    sleep_us(100);
  }
}

int main() {
  set_sys_clock_khz(130000, true);

  gpio_init(EMERGENCY_BOOTSEL_PIN);
  gpio_set_dir(EMERGENCY_BOOTSEL_PIN, GPIO_IN);
  // reboot in BOOTSEL mode if start is held
  if (gpio_get(EMERGENCY_BOOTSEL_PIN)) {
      reset_usb_boot(0, 0);
  }

  while(1) {
    tight_loop_contents();
  }

  multicore_lockout_victim_init();
  multicore_launch_core1(second_core);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
  gpio_put(PICO_DEFAULT_LED_PIN, false);

  joybus_init_comms();
  usb_init_comms();

  while (1) {
    usb_run_comms();
  }

  return 0;
}
