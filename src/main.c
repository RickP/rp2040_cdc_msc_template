#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    printf("Setup Done");

    while (true) {
      sleep_ms(1000);
      printf("Ping!");
    }
}
