#include "Puzzles/puzzle_handler.h"

#include "Puzzles/puzzle1_math.h"
#include "Puzzles/puzzle2_morsecode.h"
#include "Puzzles/puzzle3_code.h"
#include "Puzzles/puzzle4_sequence.h"
#include "Puzzles/puzzle5_color.h"

void puzzle_handler_start(puzzle_id_t puzzle)
{
    switch (puzzle)
    {
        case PUZZLE_1:
            puzzle1_math_start();
            break;

        case PUZZLE_2:
            puzzle2_morsecode_start();
            break;

        case PUZZLE_3:
            puzzle3_code_start();
            break;

        case PUZZLE_4:
            puzzle4_sequence_start();
            break;

        case PUZZLE_5:
            puzzle5_color_start();
            break;

        default:
            break;
    }
}

puzzle_status_t puzzle_handler_update(puzzle_id_t puzzle)
{
    switch (puzzle)
    {
        case PUZZLE_1:
            return puzzle1_math_update();

        case PUZZLE_2:
            return puzzle2_morsecode_update();

        case PUZZLE_3:
            return puzzle3_code_update();

        case PUZZLE_4:
            return puzzle4_sequence_update();

        case PUZZLE_5:
            return puzzle5_color_update();

        default:
            return PUZZLE_STATUS_RUNNING;
    }
}

void puzzle_handler_handle_key(puzzle_id_t puzzle, char key)
{
    switch (puzzle)
    {
        case PUZZLE_1:
            puzzle1_math_handle_key(key);
            break;

        case PUZZLE_2:
            puzzle2_morsecode_handle_key(key);
            break;

        case PUZZLE_3:
            puzzle3_code_handle_key(key);
            break;

        default:
            break;
    }
}

void puzzle_handler_handle_button(puzzle_id_t puzzle, uint8_t button)
{
    switch (puzzle)
    {
        case PUZZLE_2:
            puzzle2_morsecode_handle_button(button);
            break;

        case PUZZLE_4:
            puzzle4_sequence_handle_button(button);
            break;

        case PUZZLE_5:
            puzzle5_color_handle_button(button);
            break;

        default:
            break;
    }
}