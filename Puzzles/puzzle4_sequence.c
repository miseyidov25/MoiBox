#include "Puzzles/puzzle4_sequence.h"

#include "serial.h"
#include "HAL/Display/oled.h"

#define SEQUENCE_LENGTH 4

static const uint8_t sequence[SEQUENCE_LENGTH] =
{
    0, 2, 1, 3
};

static uint8_t input_index = 0;
static puzzle_status_t status = PUZZLE_STATUS_RUNNING;

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
}

static void print_button_name(uint8_t button)
{
    if (button == 0)
    {
        print_serial("RED");
    }
    else if (button == 1)
    {
        print_serial("GREEN");
    }
    else if (button == 2)
    {
        print_serial("BLUE");
    }
    else if (button == 3)
    {
        print_serial("YELLOW");
    }
    else
    {
        print_serial("UNKNOWN");
    }
}

static void show_sequence(void)
{
    print_serial("Sequence: RED BLUE GREEN YELLOW\r\n");

    oled_display_string(1, 0, "R B G Y");
}

void puzzle4_sequence_start(void)
{
    status = PUZZLE_STATUS_RUNNING;
    input_index = 0;

    oled_clear();
    oled_display_string(0, 0, "PUZZLE 4");
    oled_display_string(1, 0, "Repeat LEDs");

    print_serial("\r\n========== PUZZLE 4: SEQUENCE ==========\r\n");
    print_serial("Press buttons in shown sequence.\r\n");

    show_sequence();
}

puzzle_status_t puzzle4_sequence_update(void)
{
    return status;
}

void puzzle4_sequence_handle_button(uint8_t button)
{
    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

    print_serial("Pressed: ");
    print_button_name(button);
    print_serial("\r\n");

    if (button == sequence[input_index])
    {
        input_index++;

        if (input_index >= SEQUENCE_LENGTH)
        {
            print_serial("Correct sequence! Puzzle 4 solved.\r\n");

            oled_clear();
            oled_display_string(0, 0, "PUZZLE 4 SOLVED");

            status = PUZZLE_STATUS_SOLVED;
        }
    }
    else
    {
        print_serial("Wrong sequence. Restart.\r\n");

        oled_clear();
        oled_display_string(0, 0, "Wrong!");
        oled_display_string(1, 0, "Try again");

        input_index = 0;
        show_sequence();
    }
}