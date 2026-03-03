#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>

/*
 * RGB LED & STATUS LED CONTROL
 * ============================
 * 
 * FRDM-MCXA153 LED Connections:
 * 
 * RGB LEDs (5x for puzzles 1-5):
 *   LED 1: Red=PD0(J2-9), Green=PD1(J2-8), Blue=PD2(J2-7)
 *   LED 2: Red=PD3(J2-6), Green=PD4(J2-5), Blue=PD5(J2-4)
 *   LED 3: Red=PC0(J2-3), Green=PC1(J2-2), Blue=PC2(J2-1)
 *   LED 4: Red=PC3(J3-1), Green=PC4(J3-2), Blue=PC5(J3-3)
 *   LED 5: Red=PE0(J3-4), Green=PE1(J3-5), Blue=PE2(J3-6)
 * 
 * Status LEDs:
 *   LED_POWER (green): PE3 (J4 Pin 14) - System power indicator
 *   LED_ERROR (red):   PB0 (J4 Pin 12) - Error state indicator
 * 
 * Physical Connection:
 *   RGB LED cathode (color pin) --> GPIO pin (3.3V output)
 *   RGB LED anode (common+) --> 3.3V VCC through resistor (~220Ω)
 * 
 * Color Logic (GPIO outputs):\n *   RED    = R:1, G:0, B:0
 *   GREEN  = R:0, G:1, B:0
 *   YELLOW = R:1, G:1, B:0  (Red + Green combined)
 *   OFF    = R:0, G:0, B:0
 * 
 * LED Status Meanings in Puzzle Box:
 *   - YELLOW: Current puzzle waiting for signal
 *   - GREEN:  Correct signal received, puzzle solved
 *   - RED:    Wrong signal or out-of-order attempt
 */

typedef enum {
    COLOR_OFF,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW
} led_color_t;

void leds_init(void);
void rgb_led_set(uint8_t led_id, led_color_t color);
void status_led_on(uint8_t led_id);
void status_led_off(uint8_t led_id);

#endif // LEDS_H
