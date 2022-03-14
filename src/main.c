#include <stdio.h>
#include "pico/stdlib.h"
#include "usb_service.h"

int main() {
    stdio_init_all();
    usb_init();

    printf("Setup Done");

    while (true) {
      sleep_ms(1000);
      printf("Ping!\n");
    }
}
