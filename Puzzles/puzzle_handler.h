#ifndef PUZZLE_HANDLER_H
#define PUZZLE_HANDLER_H

#include <stdint.h>

#include "App/states.h"
#include "Puzzles/puzzle.h"

void puzzle_handler_start(puzzle_id_t puzzle);
puzzle_status_t puzzle_handler_update(puzzle_id_t puzzle);
void puzzle_handler_handle_key(puzzle_id_t puzzle, char key);
void puzzle_handler_handle_button(puzzle_id_t puzzle, uint8_t button);

#endif