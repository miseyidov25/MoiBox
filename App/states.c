#include "App/states.h"

static const char *stateEnumToText[] =
{
    "STATE_BOOT",
    "STATE_WAIT_FOR_LOCATION",
    "STATE_WRONG_LOCATION",
    "STATE_PUZZLE_ACTIVE",
    "STATE_PUZZLE_SOLVED",
    "STATE_ALL_SOLVED",
    "STATE_UNLOCKED",
    "STATE_RESET"
};

static const char *puzzleEnumToText[] =
{
    "PUZZLE_1",
    "PUZZLE_2",
    "PUZZLE_3",
    "PUZZLE_4",
    "PUZZLE_5",
    "PUZZLE_COUNT"
};

#define NUM_STATES  (sizeof(stateEnumToText) / sizeof(stateEnumToText[0]))
#define NUM_PUZZLES (sizeof(puzzleEnumToText) / sizeof(puzzleEnumToText[0]))

const char *state_to_string(app_state_t state)
{
    if ((int)state >= 0 && (unsigned int)state < NUM_STATES)
    {
        return stateEnumToText[state];
    }

    return "UNKNOWN_STATE";
}

const char *puzzle_to_string(puzzle_id_t puzzle)
{
    if ((int)puzzle >= 0 && (unsigned int)puzzle < NUM_PUZZLES)
    {
        return puzzleEnumToText[puzzle];
    }

    return "UNKNOWN_PUZZLE";
}