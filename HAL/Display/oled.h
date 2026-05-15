#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include <stdbool.h>

// Function declarations
void oled_init(void);
void oled_clear(void);
void oled_display_string(uint8_t row, uint8_t col, const char *str);
void oled_display_value(uint8_t row, uint8_t col, int32_t value);
void oled_display_puzzle_state(const char *puzzle_name, const char *state);
void oled_update(void);

#endif // OLED_H
