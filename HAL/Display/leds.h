#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>
#include <stdbool.h>

void leds_init(void);
void leds_update(uint32_t current_ms);

/*
 * Normal location LED logic:
 * red      = not next / not completed
 * yellow   = next location
 * green    = completed
 * blinking = current active/next location
 */
void leds_update_map(uint8_t current_puzzle, const bool solved[5]);
void leds_set_active_puzzle(uint8_t puzzle_number);

void leds_set_wrong_location_flash(uint8_t puzzle_number);

#endif