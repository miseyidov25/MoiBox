#include <MCXA153.h>
#include <stdint.h>

#include "Puzzles/puzzle1_math.h"

#include "App/settings.h"

#include "serial.h"
#include "HAL/Display/oled.h"

#define MAX_INPUT 4

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;

static char input[MAX_INPUT + 1];
static int input_index = 0;

static int apple_value = 0;
static int banana_value = 0;
static int x_value = 0;
static int y_value = 0;
static int correct_answer = 0;

static char line1[32];
static char line2[32];
static char line3[32];

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
}

static void print_int(int value)
{
    char buffer[12];
    int index = 0;

    if (value == 0)
    {
        serial_putchar('0');
        return;
    }

    if (value < 0)
    {
        serial_putchar('-');
        value = -value;
    }

    while (value > 0 && index < 11)
    {
        buffer[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index > 0)
    {
        serial_putchar(buffer[--index]);
    }
}

static void int_to_string(int value, char *buffer)
{
    char temp[12];
    int index = 0;
    int out = 0;

    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (value < 0)
    {
        buffer[out++] = '-';
        value = -value;
    }

    while (value > 0 && index < 11)
    {
        temp[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index > 0)
    {
        buffer[out++] = temp[--index];
    }

    buffer[out] = '\0';
}

static void make_equation_line(char *buffer, const char *left, int result)
{
    char result_text[12];
    int i = 0;
    int j = 0;

    int_to_string(result, result_text);

    while (left[i] != '\0')
    {
        buffer[i] = left[i];
        i++;
    }

    buffer[i++] = '=';

    while (result_text[j] != '\0')
    {
        buffer[i++] = result_text[j++];
    }

    buffer[i] = '\0';
}

static uint32_t randomish(void)
{
    /*
     * SysTick->VAL changes while the program is running.
     * Good enough for puzzle variation.
     */
    return SysTick->VAL;
}

static void generate_problem(void)
{
    uint32_t r = randomish();

    apple_value = (int)((r % 9u) + 1u);
    banana_value = (int)(((r >> 5u) % 9u) + 1u);

    y_value = apple_value + apple_value;

    if (app_settings_get_difficulty() == APP_DIFFICULTY_EASY)
    {
        /*
         * Easy:
         * Banana + Apple = X
         * Apple + Apple = Y
         * X = ?
         */
        x_value = banana_value + apple_value;
    }
    else
    {
        /*
         * Hard:
         * Banana * Apple = X
         * Apple + Apple = Y
         * X = ?
         */
        x_value = banana_value * apple_value;
    }

    correct_answer = x_value;

    if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
    {
        if (app_settings_get_difficulty() == APP_DIFFICULTY_EASY)
        {
            make_equation_line(line1, "Banaan+Appel", x_value);
        }
        else
        {
            make_equation_line(line1, "Banaan*Appel", x_value);
        }

        make_equation_line(line2, "Appel+Appel", y_value);
    }
    else
    {
        if (app_settings_get_difficulty() == APP_DIFFICULTY_EASY)
        {
            make_equation_line(line1, "Banana+Apple", x_value);
        }
        else
        {
            make_equation_line(line1, "Banana*Apple", x_value);
        }

        make_equation_line(line2, "Apple+Apple", y_value);
    }

    line3[0] = 'X';
    line3[1] = ' ';
    line3[2] = '=';
    line3[3] = ' ';
    line3[4] = '?';
    line3[5] = '\0';
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
    generate_problem();

    oled_clear();

    if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
    {
        oled_display_string(0, 0, "PUZZEL 1");
    }
    else
    {
        oled_display_string(0, 0, "PUZZLE 1");
    }

    oled_display_string(1, 0, line1);
    oled_display_string(2, 0, line2);
    oled_display_string(3, 0, line3);

    print_serial("\r\n========== PUZZLE 1: MATH ==========\r\n");

    print_serial("Difficulty: ");
    print_serial(app_settings_difficulty_to_string(app_settings_get_difficulty()));
    print_serial("\r\n");

    print_serial("Language: ");
    print_serial(app_settings_language_to_string(app_settings_get_language()));
    print_serial("\r\n");

    print_serial(line1);
    print_serial("\r\n");
    print_serial(line2);
    print_serial("\r\n");
    print_serial(line3);
    print_serial("\r\n");

    print_serial("\r\nDebug values:\r\n");
    print_serial("Apple = ");
    print_int(apple_value);
    print_serial("\r\n");

    print_serial("Banana = ");
    print_int(banana_value);
    print_serial("\r\n");

    print_serial("X = ");
    print_int(x_value);
    print_serial("\r\n");

    print_serial("Y = ");
    print_int(y_value);
    print_serial("\r\n");

    print_serial("\r\n# = enter, * = delete\r\n");
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

            if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
            {
                print_serial("\r\nVerwijderd\r\nInput: ");
            }
            else
            {
                print_serial("\r\nDeleted\r\nInput: ");
            }
        }

        return;
    }

    if (key == '#')
    {
        int answer = string_to_int(input);

        print_serial("\r\n");

        if (answer == correct_answer)
        {
            if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
            {
                print_serial("Correct! Puzzel 1 opgelost.\r\n");

                oled_clear();
                oled_display_string(0, 0, "PUZZEL 1 KLAAR");
            }
            else
            {
                print_serial("Correct! Puzzle 1 solved.\r\n");

                oled_clear();
                oled_display_string(0, 0, "PUZZLE 1 SOLVED");
            }

            status = PUZZLE_STATUS_SOLVED;
        }
        else
        {
            if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
            {
                print_serial("Fout. Probeer opnieuw.\r\n");
            }
            else
            {
                print_serial("Wrong answer. Try again.\r\n");
            }

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