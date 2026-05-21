#include "Puzzles/puzzle2_morsecode.h"

#include "serial.h"
#include "HAL/Display/oled.h"
#include "HAL/Audio/buzzer.h"

#define REQUIRED_PRESSES 3

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;
static uint8_t press_count = 0;
static uint8_t clue_played = 0;

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
}

static void show_progress(void)
{
    print_serial("Morse S progress: ");

    for (uint8_t i = 0; i < REQUIRED_PRESSES; i++)
    {
        if (i < press_count)
        {
            print_serial(". ");
        }
        else
        {
            print_serial("_ ");
        }
    }

    print_serial("\r\n");
}

void puzzle2_morsecode_start(void)
{
    status = PUZZLE_STATUS_RUNNING;
    press_count = 0;
    clue_played = 0;

    oled_clear();
    oled_display_string(0, 0, "PUZZLE 2");
    oled_display_string(1, 0, "Morse S");
    oled_display_string(2, 0, ". . .");

    print_serial("\r\n========== PUZZLE 2: MORSE ==========\r\n");
    print_serial("S = . . .\r\n");
    print_serial("Press any color button 3 times.\r\n");
    print_serial("Press # to replay the sound.\r\n");
}

puzzle_status_t puzzle2_morsecode_update(void)
{
    if (!clue_played)
    {
        clue_played = 1;
        buzzer_morse_S();
        show_progress();
    }

    return status;
}

void puzzle2_morsecode_handle_key(char key)
{
    if (key == '#')
    {
        buzzer_morse_S();
    }
}

void puzzle2_morsecode_handle_button(uint8_t button)
{
    (void)button;

    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

    press_count++;
    buzzer_click();
    show_progress();

    if (press_count == REQUIRED_PRESSES)
    {
        print_serial("Correct! Puzzle 2 solved.\r\n");
        buzzer_success();

        oled_clear();
        oled_display_string(0, 0, "PUZZLE 2 SOLVED");

        status = PUZZLE_STATUS_SOLVED;
    }
    else if (press_count > REQUIRED_PRESSES)
    {
        print_serial("Too many presses. Try again.\r\n");
        buzzer_fail();
        press_count = 0;
        show_progress();
    }
}