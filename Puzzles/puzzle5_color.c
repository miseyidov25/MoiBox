#include "Puzzles/puzzle5_color.h"

#include "serial.h"
#include "HAL/Display/oled.h"

#define COLOR_RED     0
#define COLOR_GREEN   1
#define COLOR_BLUE    2
#define COLOR_YELLOW  3

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;
static uint8_t correct_button = COLOR_BLUE;

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
}

static void print_button_name(uint8_t button)
{
    if (button == COLOR_RED)
    {
        print_serial("RED");
    }
    else if (button == COLOR_GREEN)
    {
        print_serial("GREEN");
    }
    else if (button == COLOR_BLUE)
    {
        print_serial("BLUE");
    }
    else if (button == COLOR_YELLOW)
    {
        print_serial("YELLOW");
    }
    else
    {
        print_serial("UNKNOWN");
    }
}

void puzzle5_color_start(void)
{
    status = PUZZLE_STATUS_RUNNING;
    correct_button = COLOR_BLUE;

    oled_clear();
    oled_display_string(0, 0, "PUZZLE 5");
    oled_display_string(1, 0, "Clue: SKY");
    oled_display_string(2, 0, "Press color");

    print_serial("\r\n========== PUZZLE 5: COLOR ==========\r\n");
    print_serial("Clue word: SKY\r\n");
    print_serial("Press matching color.\r\n");
}

puzzle_status_t puzzle5_color_update(void)
{
    return status;
}

void puzzle5_color_handle_button(uint8_t button)
{
    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

    print_serial("Pressed: ");
    print_button_name(button);
    print_serial("\r\n");

    if (button == correct_button)
    {
        print_serial("Correct color! Puzzle 5 solved.\r\n");

        oled_clear();
        oled_display_string(0, 0, "PUZZLE 5 SOLVED");

        status = PUZZLE_STATUS_SOLVED;
    }
    else
    {
        print_serial("Wrong color. Try again.\r\n");

        oled_clear();
        oled_display_string(0, 0, "Wrong color");
        oled_display_string(1, 0, "Clue: SKY");
    }
}