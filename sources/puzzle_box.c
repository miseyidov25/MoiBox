#include "puzzle_box.h"

static uint8_t current_puzzle = 0;

void puzzlebox_init(void) {
    current_puzzle = 0;
}

void puzzlebox_update(void) {
    // placeholder for state transitions
}

void puzzlebox_reset(void) {
    current_puzzle = 0;
}

uint8_t puzzlebox_get_current_puzzle(void) {
    return current_puzzle;
}

uint8_t puzzlebox_is_complete(void) {
    return current_puzzle >= NUM_PUZZLES;
}

void puzzlebox_signal_received(uint8_t puzzle_id) {
    (void)puzzle_id;
}
