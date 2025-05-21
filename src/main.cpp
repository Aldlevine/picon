/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cstdio>

#include <pico/stdlib.h>

// #include <cstdio>

#ifndef PICO_DEFAULT_LED_PIN
#error requires a board with a regular LED
#endif

constexpr auto LED_OFF_MS = 1000;
constexpr auto LED_ON_MS = 100;
constexpr auto LED_PIN = PICO_DEFAULT_LED_PIN;

// Initialize the GPIO for the LED
void pico_led_init(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

// Turn the LED on or off
void pico_set_led(bool led_on) {
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
}

int main() {
    int step = 0;

    stdio_init_all();

    pico_led_init();
    while (true) {
        printf("Blink %d\n", step++);
        pico_set_led(true);
        sleep_ms(LED_ON_MS);
        pico_set_led(false);
        sleep_ms(LED_OFF_MS);
    }
}
