#ifndef PUZZLE_BOX_H
#define PUZZLE_BOX_H

#include <stdint.h>


void puzzlebox_init(void);                  // start/clear state
void puzzlebox_update(void);                // called periodically
void puzzlebox_reset(void);                 // reset to initial state

uint8_t puzzlebox_get_current_puzzle(void); // 0-based index
uint8_t puzzlebox_is_complete(void);        // non-zero when done

void puzzlebox_signal_received(uint8_t puzzle_id); // input handler

#endif // PUZZLE_BOX_H
