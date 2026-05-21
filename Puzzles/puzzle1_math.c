#include "Puzzles/puzzle1_math.h"

#include "serial.h"
#include "HAL/Display/oled.h"

#define MAX_INPUT 4

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;
static char input[MAX_INPUT + 1];
static int input_index = 0;

static const int correct_answer = 4;

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
}

static void reset_input(void)
{
    for (int i = 0; i <= MAX_INPUT; i++)
    {
        input[i] = 0;
    }

    input_index = 0;
}

static int string_to_int(const char *s)
{
    int value = 0;

    if (*s == 0)
    {
        return -1;
    }

    while (*s)
    {
        if (*s < '0' || *s > '9')
        {
            return -1;
        }

        value = value * 10 + (*s - '0');
        s++;
    }

    return value;
}

void puzzle1_math_start(void)
{
    status = PUZZLE_STATUS_RUNNING;
    reset_input();

    oled_clear();
    oled_display_string(0, 0, "PUZZLE 1");
    oled_display_string(1, 0, "x + 3 = 7");
    oled_display_string(2, 0, "3 + 3 = 6");
    oled_display_string(3, 0, "Find x");

    print_serial("\r\n========== PUZZLE 1: MATH ==========\r\n");
    print_serial("x + 3 = 7\r\n");
    print_serial("3 + 3 = 6\r\n");
    print_serial("Find x\r\n");
    print_serial("# = enter, * = delete\r\n");
    print_serial("Input: ");
}

puzzle_status_t puzzle1_math_update(void)
{
    return status;
}

void puzzle1_math_handle_key(char key)
{
    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

    if (key == '*')
    {
        if (input_index > 0)
        {
            input_index--;
            input[input_index] = 0;
            print_serial("\r\nDeleted\r\nInput: ");
        }

        return;
    }

    if (key == '#')
    {
        int answer = string_to_int(input);

        print_serial("\r\n");

        if (answer == correct_answer)
        {
            print_serial("Correct! Puzzle 1 solved.\r\n");

            oled_clear();
            oled_display_string(0, 0, "PUZZLE 1 SOLVED");

            status = PUZZLE_STATUS_SOLVED;
        }
        else
        {
            print_serial("Wrong answer. Try again.\r\n");
            reset_input();
            print_serial("Input: ");
        }

        return;
    }

    if (key < '0' || key > '9')
    {
        return;
    }

    if (input_index >= MAX_INPUT)
    {
        return;
    }

    input[input_index++] = key;
    serial_putchar(key);
}