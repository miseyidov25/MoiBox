#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>
#include <stdbool.h>

void leds_init(void);
void leds_update(uint32_t current_ms);

/*
 * Location RGB LED logic:
 *
 * red      = not next / not completed
 * yellow   = next location
 * green    = completed
 * blinking = current active/next location
 */
void leds_update_map(uint8_t current_puzzle, const bool solved[5]);
void leds_set_active_puzzle(uint8_t puzzle_number);
void leds_set_wrong_location_flash(uint8_t puzzle_number);
void leds_set_current_location_blink(uint8_t puzzle_number);

/*
 * Normal colored LEDs.
 *
 * These are separate from the 5 location RGB map LEDs.
 *
 * Green  -> P3_6
 * Blue   -> P3_7
 * Yellow -> P3_30
 * Red    -> P1_12
 */
void leds_normal_all_off(void);

void leds_normal_red_on(void);
void leds_normal_green_on(void);
void leds_normal_blue_on(void);
void leds_normal_yellow_on(void);

void leds_normal_red_off(void);
void leds_normal_green_off(void);
void leds_normal_blue_off(void);
void leds_normal_yellow_off(void);

void leds_normal_red_toggle(void);
void leds_normal_green_toggle(void);
void leds_normal_blue_toggle(void);
void leds_normal_yellow_toggle(void);

void leds_normal_set(uint8_t red, uint8_t green, uint8_t blue, uint8_t yellow);

#endif