#include "leds.h"
#include "pins.h"

void main(void) {
    // Initialize hardware components
    leds_init();
    buzzer_init();
    switches_init();
    oled_init();
    

    while (1) {
        // Main loop
    }
}