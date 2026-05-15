#ifndef OLED_H
#define OLED_H

#include <stdint.h>

// (I2C/SSD1306)


void oled_init(void);                         // initialize display
void oled_display_puzzle(uint8_t puzzle_num); // show puzzle number
void oled_display_status(const char *status); // show text message

#endif // OLED_H
