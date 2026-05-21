#ifndef STATES_H
#define STATES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    PUZZLE_1 = 0,
    PUZZLE_2,
    PUZZLE_3,
    PUZZLE_4,
    PUZZLE_5,
    PUZZLE_COUNT
} puzzle_id_t;

typedef enum
{
    STATE_BOOT = 0,
    STATE_WAIT_FOR_LOCATION,
    STATE_WRONG_LOCATION,
    STATE_PUZZLE_ACTIVE,
    STATE_PUZZLE_SOLVED,
    STATE_ALL_SOLVED,
    STATE_UNLOCKED,
    STATE_RESET
} app_state_t;

const char *state_to_string(app_state_t state);
const char *puzzle_to_string(puzzle_id_t puzzle);

#endif