#include <stdint.h>

#include "Puzzles/puzzle2_morsecode.h"

#include "serial.h"
#include "HAL/Display/oled.h"
#include "HAL/Audio/buzzer.h"

/*
 * Change difficulty here:
 *
 * EASY = Morse spells a color word: RED / GREEN / BLUE / YELLOW
 * HARD = Morse spells a 3-digit number code
 */
#define PUZZLE2_DIFFICULTY PUZZLE2_DIFFICULTY_EASY

#define NUMBER_CODE_LENGTH 3u

#define COLOR_RED     0u
#define COLOR_GREEN   1u
#define COLOR_BLUE    2u
#define COLOR_YELLOW  3u

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;
static uint8_t clue_played = 0u;

#if PUZZLE2_DIFFICULTY == PUZZLE2_DIFFICULTY_HARD

static const char number_code[NUMBER_CODE_LENGTH + 1u] =
{
    '7', '0', '5', '\0'
};

static char input[NUMBER_CODE_LENGTH + 1u];
static uint8_t input_index = 0u;

#endif

#if PUZZLE2_DIFFICULTY == PUZZLE2_DIFFICULTY_EASY

/*
 * For easy mode, change this variable:
 *
 * COLOR_RED
 * COLOR_GREEN
 * COLOR_BLUE
 * COLOR_YELLOW
 */
static uint8_t correct_color = COLOR_BLUE;

static const char *color_words[] =
{
    "RED",
    "GREEN",
    "BLUE",
    "YELLOW"
};

#endif

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
}

#if PUZZLE2_DIFFICULTY == PUZZLE2_DIFFICULTY_HARD

static void reset_input(void)
{
    for (uint8_t i = 0u; i <= NUMBER_CODE_LENGTH; i++)
    {
        input[i] = 0;
    }

    input_index = 0u;
}

static int number_answer_correct(void)
{
    for (uint8_t i = 0u; i < NUMBER_CODE_LENGTH; i++)
    {
        if (input[i] != number_code[i])
        {
            return 0;
        }
    }

    return 1;
}

#endif

static void play_current_clue(void)
{
#if PUZZLE2_DIFFICULTY == PUZZLE2_DIFFICULTY_EASY

    print_serial("Playing Morse color clue: ");
    print_serial(color_words[correct_color]);
    print_serial("\r\n");

    buzzer_morse_string(color_words[correct_color]);

#else

    print_serial("Playing Morse number clue: ");
    print_serial(number_code);
    print_serial("\r\n");

    buzzer_morse_string(number_code);

#endif
}

void puzzle2_morsecode_start(void)
{
    status = PUZZLE_STATUS_RUNNING;
    clue_played = 0u;

#if PUZZLE2_DIFFICULTY == PUZZLE2_DIFFICULTY_HARD
    reset_input();
#endif

#if PUZZLE2_DIFFICULTY == PUZZLE2_DIFFICULTY_EASY

    oled_clear();
    oled_display_string(0, 0, "PUZZLE 2");
    oled_display_string(1, 0, "Morse color");
    oled_display_string(2, 0, "Press color");

    print_serial("\r\n========== PUZZLE 2: MORSE ==========\r\n");
    print_serial("Difficulty: EASY\r\n");
    print_serial("Listen to Morse, press matching color button.\r\n");
    print_serial("Press # to replay.\r\n");

#else

    oled_clear();
    oled_display_string(0, 0, "PUZZLE 2");
    oled_display_string(1, 0, "Morse number");
    oled_display_string(2, 0, "Enter + #");

    print_serial("\r\n========== PUZZLE 2: MORSE ==========\r\n");
    print_serial("Difficulty: HARD\r\n");
    print_serial("Listen to Morse, enter 3 digits, press #.\r\n");
    print_serial("Press # with empty input to replay.\r\n");
    print_serial("Input: ");

#endif
}

puzzle_status_t puzzle2_morsecode_update(void)
{
    if (!clue_played)
    {
        clue_played = 1u;
        play_current_clue();
    }

    return status;
}

void puzzle2_morsecode_handle_key(char key)
{
    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

#if PUZZLE2_DIFFICULTY == PUZZLE2_DIFFICULTY_EASY

    if (key == '#')
    {
        play_current_clue();
    }

    (void)key;

#else

    if (key == '#')
    {
        if (input_index == 0u)
        {
            play_current_clue();
            print_serial("Input: ");
            return;
        }

        if (input_index != NUMBER_CODE_LENGTH)
        {
            print_serial("\r\nEnter exactly 3 digits.\r\nInput: ");
            return;
        }

        print_serial("\r\n");

        if (number_answer_correct())
        {
            print_serial("Correct Morse number! Puzzle 2 solved.\r\n");
            buzzer_success();

            oled_clear();
            oled_display_string(0, 0, "PUZZLE 2 SOLVED");

            status = PUZZLE_STATUS_SOLVED;
        }
        else
        {
            print_serial("Wrong number. Try again.\r\n");
            buzzer_fail();
            reset_input();
            print_serial("Input: ");
        }

        return;
    }

    if (key == '*')
    {
        if (input_index > 0u)
        {
            input_index--;
            input[input_index] = 0;
            print_serial("\r\nDeleted\r\nInput: ");
        }

        return;
    }

    if (key < '0' || key > '9')
    {
        return;
    }

    if (input_index >= NUMBER_CODE_LENGTH)
    {
        return;
    }

    input[input_index] = key;
    input_index++;

    serial_putchar(key);
    serial_putchar(' ');

#endif
}

void puzzle2_morsecode_handle_button(uint8_t button)
{
    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

#if PUZZLE2_DIFFICULTY == PUZZLE2_DIFFICULTY_EASY

    if (button == correct_color)
    {
        print_serial("Correct color! Puzzle 2 solved.\r\n");
        buzzer_success();

        oled_clear();
        oled_display_string(0, 0, "PUZZLE 2 SOLVED");

        status = PUZZLE_STATUS_SOLVED;
    }
    else
    {
        print_serial("Wrong color. Listen again.\r\n");
        buzzer_fail();
        play_current_clue();
    }

#else

    /*
     * In hard mode, colored buttons are ignored.
     */
    (void)button;

#endif
}