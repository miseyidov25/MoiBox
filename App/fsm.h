#ifndef FSM_H
#define FSM_H

#include <stdbool.h>
#include "App/events.h"
#include "App/states.h"

void fsm_init(void);
void fsm_update(void);
void fsm_handle_event(app_event_t event);

app_state_t fsm_get_state(void);
puzzle_id_t fsm_get_current_puzzle(void);
bool fsm_is_puzzle_solved(puzzle_id_t puzzle);

void fsm_set_puzzle_for_location(uint8_t location_number, puzzle_id_t puzzle);
puzzle_id_t fsm_get_puzzle_for_location(uint8_t location_number);
uint8_t fsm_get_current_location(void);

void fsm_set_puzzle_order(
    uint8_t p1,
    uint8_t p2,
    uint8_t p3,
    uint8_t p4,
    uint8_t p5
);

#endif