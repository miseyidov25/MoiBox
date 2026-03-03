#include "leds.h"
#include "pins.h"

// mapping from led index to its R/G/B pin constants
static const int rgb_pins[5][3] = {
    {RGB_LED1_R, RGB_LED1_G, RGB_LED1_B},
    {RGB_LED2_R, RGB_LED2_G, RGB_LED2_B},
    {RGB_LED3_R, RGB_LED3_G, RGB_LED3_B},
    {RGB_LED4_R, RGB_LED4_G, RGB_LED4_B},
    {RGB_LED5_R, RGB_LED5_G, RGB_LED5_B}
};

// colors mapped to RGB bit patterns
static const uint8_t color_bits[][3] = {
    {0,0,0}, // OFF
    {1,0,0}, // RED
    {0,1,0}, // GREEN
    {1,1,0}  // YELLOW
};

static void pinMode(uint8_t port, uint8_t pin, uint8_t mode) { (void)port; (void)pin; (void)mode; }
static void digitalWrite(uint8_t port, uint8_t pin, uint8_t val) { (void)port; (void)pin; (void)val; }

void leds_init(void) {
    // configure all LED pins as outputs and turn off
    for (int led = 0; led < 5; led++) {
        pinMode(rgb_pins[led][0], 1);
        pinMode(rgb_pins[led][1], 1);
        pinMode(rgb_pins[led][2], 1);
        rgb_led_set(led, COLOR_OFF);
    }
}

void rgb_led_set(uint8_t led_id, led_color_t color) {
    if (led_id >= 5 || color > COLOR_YELLOW) return;
    for (int i = 0; i < 3; i++) {
        digitalWrite(rgb_pins[led_id][i], color_bits[color][i]);
    }
}

void status_led_on(uint8_t led_id) {
    // just set the pin high (simple)
    digitalWrite(LED_POWER, 1);
    (void)led_id;
}

void status_led_off(uint8_t led_id) {
    digitalWrite(LED_POWER, 0);
    (void)led_id;
}

