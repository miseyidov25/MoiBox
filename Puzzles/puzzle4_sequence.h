#ifndef PUZZLE4_SEQUENCE_H
#define PUZZLE4_SEQUENCE_H

#include <stdint.h>

#include "Puzzles/puzzle.h"

void puzzle4_sequence_start(void);
puzzle_status_t puzzle4_sequence_update(void);
void puzzle4_sequence_handle_button(uint8_t button);

#endif