#ifndef PUZZLE5_COLOR_H
#define PUZZLE5_COLOR_H

#include <stdint.h>

#include "Puzzles/puzzle.h"

void puzzle5_color_start(void);
puzzle_status_t puzzle5_color_update(void);
void puzzle5_color_handle_button(uint8_t button);

#endif