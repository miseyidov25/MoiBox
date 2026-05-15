#include "leds.h"
#include "pins.h"

// Placeholder implementation - actual LED functionality is in HAL/Display/leds.h
// Note: Pin macros now use port/pin pairs instead of GPIO constants
// so array initialization is commented out

// colors mapped to RGB bit patterns - currently unused
// static const uint8_t color_bits[][3] = {
//     {0,0,0}, // OFF
//     {1,0,0}, // RED
//     {0,1,0}, // GREEN
//     {1,1,0}  // YELLOW
// };

// simple placeholders for hardware control - currently unused
// static void pinMode(uint8_t port, uint8_t pin, uint8_t mode) { (void)port; (void)pin; (void)mode; }
// static void digitalWrite(uint8_t port, uint8_t pin, uint8_t val) { (void)port; (void)pin; (void)val; }

void leds_init(void) {
    // Placeholder - actual LED initialization is in HAL
}

// Functions below are stubs for compatibility
void rgb_led_set(uint8_t led_id, uint8_t color) {
    (void)led_id;
    (void)color;
}

void status_led_on(uint8_t led_id) {
    (void)led_id;
}

void status_led_off(uint8_t led_id) {
    (void)led_id;
}
