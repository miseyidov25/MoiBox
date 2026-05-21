#include "Puzzles/puzzle3_code.h"

#include "serial.h"
#include "HAL/Display/oled.h"

#define CODE_LENGTH 7

static const char correct_code[CODE_LENGTH] =
{
    '1','3','4','6','7','9','1'
};

static char user_input[CODE_LENGTH];
static int input_index = 0;
static puzzle_status_t status = PUZZLE_STATUS_RUNNING;

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
}

static void reset_input(void)
{
    for (int i = 0; i < CODE_LENGTH; i++)
    {
        user_input[i] = 0;
    }

    input_index = 0;
}

static int code_is_correct(void)
{
    for (int i = 0; i < CODE_LENGTH; i++)
    {
        if (user_input[i] != correct_code[i])
        {
            return 0;
        }
    }

    return 1;
}

void puzzle3_code_start(void)
{
    reset_input();
    status = PUZZLE_STATUS_RUNNING;

    oled_clear();
    oled_display_string(0, 0, "PUZZLE 3");
    oled_display_string(1, 0, "Enter 7 digits");
    oled_display_string(2, 0, "# enter");
    oled_display_string(3, 0, "* delete");

    print_serial("\r\n========== PUZZLE 3: CODE ==========\r\n");
    print_serial("Enter the 7-digit code\r\n");
    print_serial("# = ENTER\r\n");
    print_serial("* = BACKSPACE\r\n");
    print_serial("Input: ");
}

puzzle_status_t puzzle3_code_update(void)
{
    return status;
}

void puzzle3_code_handle_key(char key)
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
            user_input[input_index] = 0;
            print_serial("\r\nDeleted\r\nInput: ");
        }

        return;
    }

    if (key == '#')
    {
        if (input_index != CODE_LENGTH)
        {
            print_serial("\r\nCode must contain 7 digits.\r\nInput: ");
            return;
        }

        print_serial("\r\n");

        if (code_is_correct())
        {
            print_serial("\r\nCORRECT CODE!\r\n");
            print_serial("Puzzle 3 solved.\r\n");

            oled_clear();
            oled_display_string(0, 0, "PUZZLE 3 SOLVED");

            status = PUZZLE_STATUS_SOLVED;
        }
        else
        {
            print_serial("\r\nWRONG CODE!\r\n");
            print_serial("Try again.\r\n");
            reset_input();
            print_serial("Input: ");
        }

        return;
    }

    if (key < '0' || key > '9')
    {
        return;
    }

    if (input_index >= CODE_LENGTH)
    {
        return;
    }

    user_input[input_index++] = key;
    serial_putchar(key);
    serial_putchar(' ');
}