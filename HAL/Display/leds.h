#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>
#include <stdbool.h>

void leds_init(void);
void leds_update_map(uint8_t current_puzzle, bool solved[]);
void leds_all_off(void);

void leds_set_red(void);
void leds_set_yellow(void);
void leds_set_green(void);

#endif