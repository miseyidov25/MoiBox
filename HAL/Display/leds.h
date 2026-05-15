#ifndef LEDS_H
#define LEDS_H

#include <stdbool.h>
#include <stdint.h>

// Function declarations
void leds_init(void);
void leds_all_off(void);
void leds_set_red(void);
void leds_set_yellow(void);
void leds_set_blue(bool on);
void leds_set_green(void);
void leds_set_distance_color(int8_t tx_power);
void leds_set_sw3_mode(bool mode);
bool leds_get_sw3_mode(void);

#endif // LEDS_H
