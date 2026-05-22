#ifndef PUZZLE2_MORSECODE_H
#define PUZZLE2_MORSECODE_H

#include <stdint.h>
#include "Puzzles/puzzle.h"

void puzzle2_morsecode_start(void);
puzzle_status_t puzzle2_morsecode_update(void);

void puzzle2_morsecode_handle_key(char key);
void puzzle2_morsecode_handle_button(uint8_t button);

#endif