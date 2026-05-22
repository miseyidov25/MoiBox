#include "Puzzles/puzzle3_code.h"

#include "App/settings.h"

#include "serial.h"
#include "HAL/Display/oled.h"

#define EASY_CODE_LENGTH 7
#define MAX_INPUT_LENGTH 9

static const char easy_code[EASY_CODE_LENGTH + 1] =
{
    '1','3','4','6','7','9','1','\0'
};

/*
 * Hard mode:
 * User sees the first 7 numbers and must enter the 8th.
 *
 * 2, 4, 8, 16, 32, 64, 128, ?
 *
 * Correct answer = 256
 */
static const int hard_sequence[7] =
{
    2, 4, 8, 16, 32, 64, 128
};

static const int hard_answer = 256;

static char user_input[MAX_INPUT_LENGTH + 1];
static int input_index = 0;

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;

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

static int is_dutch(void)
{
    return app_settings_get_language() == APP_LANGUAGE_DUTCH;
}

static void reset_input(void)
{
    for (int i = 0; i <= MAX_INPUT_LENGTH; i++)
    {
        user_input[i] = 0;
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

static int easy_code_is_correct(void)
{
    for (int i = 0; i < EASY_CODE_LENGTH; i++)
    {
        if (user_input[i] != easy_code[i])
        {
            return 0;
        }
    }

    return 1;
}

static void print_hard_sequence(void)
{
    for (int i = 0; i < 7; i++)
    {
        print_int(hard_sequence[i]);

        if (i < 6)
        {
            print_serial(", ");
        }
    }
}

void puzzle3_code_start(void)
{
    reset_input();
    status = PUZZLE_STATUS_RUNNING;

    oled_clear();

    print_serial("\r\n========== PUZZLE 3: CODE ==========\r\n");

    print_serial("Difficulty: ");
    print_serial(app_settings_difficulty_to_string(app_settings_get_difficulty()));
    print_serial("\r\n");

    print_serial("Language: ");
    print_serial(app_settings_language_to_string(app_settings_get_language()));
    print_serial("\r\n");

    if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
    {
        if (is_dutch())
        {
            oled_display_string(0, 0, "PUZZEL 3");
            oled_display_string(1, 0, "Vind nummer 8");
            oled_display_string(2, 0, "2 4 8 16");
            oled_display_string(3, 0, "32 64 128 ?");

            print_serial("Vind het 8e nummer in de reeks.\r\n");
            print_serial("Reeks: ");
            print_hard_sequence();
            print_serial("\r\n");
            print_serial("Voer het volgende nummer in en druk #.\r\n");
        }
        else
        {
            oled_display_string(0, 0, "PUZZLE 3");
            oled_display_string(1, 0, "Find number 8");
            oled_display_string(2, 0, "2 4 8 16");
            oled_display_string(3, 0, "32 64 128 ?");

            print_serial("Find the 8th number in the sequence.\r\n");
            print_serial("Sequence: ");
            print_hard_sequence();
            print_serial("\r\n");
            print_serial("Enter the next number and press #.\r\n");
        }

        print_serial("Debug answer: ");
        print_int(hard_answer);
        print_serial("\r\n");
    }
    else
    {
        if (is_dutch())
        {
            oled_display_string(0, 0, "PUZZEL 3");
            oled_display_string(1, 0, "Voer 7 cijfers");
            oled_display_string(2, 0, "# invoer");
            oled_display_string(3, 0, "* wissen");

            print_serial("Voer de 7-cijferige code in.\r\n");
            print_serial("# = INVOER\r\n");
            print_serial("* = WISSEN\r\n");
        }
        else
        {
            oled_display_string(0, 0, "PUZZLE 3");
            oled_display_string(1, 0, "Enter 7 digits");
            oled_display_string(2, 0, "# enter");
            oled_display_string(3, 0, "* delete");

            print_serial("Enter the 7-digit code.\r\n");
            print_serial("# = ENTER\r\n");
            print_serial("* = BACKSPACE\r\n");
        }

        print_serial("Debug code: ");
        print_serial(easy_code);
        print_serial("\r\n");
    }

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

            print_serial(is_dutch() ? "\r\nVerwijderd\r\nInput: " : "\r\nDeleted\r\nInput: ");
        }

        return;
    }

    if (key == '#')
    {
        print_serial("\r\n");

        if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
        {
            int answer = string_to_int(user_input);

            if (answer == hard_answer)
            {
                if (is_dutch())
                {
                    print_serial("\r\nCORRECT!\r\n");
                    print_serial("Puzzel 3 opgelost.\r\n");

                    oled_clear();
                    oled_display_string(0, 0, "PUZZEL 3 KLAAR");
                }
                else
                {
                    print_serial("\r\nCORRECT!\r\n");
                    print_serial("Puzzle 3 solved.\r\n");

                    oled_clear();
                    oled_display_string(0, 0, "PUZZLE 3 SOLVED");
                }

                status = PUZZLE_STATUS_SOLVED;
            }
            else
            {
                if (is_dutch())
                {
                    print_serial("\r\nFOUT ANTWOORD!\r\n");
                    print_serial("Probeer opnieuw.\r\n");
                }
                else
                {
                    print_serial("\r\nWRONG ANSWER!\r\n");
                    print_serial("Try again.\r\n");
                }

                reset_input();
                print_serial("Input: ");
            }

            return;
        }

        if (input_index != EASY_CODE_LENGTH)
        {
            if (is_dutch())
            {
                print_serial("\r\nCode moet 7 cijfers hebben.\r\nInput: ");
            }
            else
            {
                print_serial("\r\nCode must contain 7 digits.\r\nInput: ");
            }

            return;
        }

        if (easy_code_is_correct())
        {
            if (is_dutch())
            {
                print_serial("\r\nCORRECTE CODE!\r\n");
                print_serial("Puzzel 3 opgelost.\r\n");

                oled_clear();
                oled_display_string(0, 0, "PUZZEL 3 KLAAR");
            }
            else
            {
                print_serial("\r\nCORRECT CODE!\r\n");
                print_serial("Puzzle 3 solved.\r\n");

                oled_clear();
                oled_display_string(0, 0, "PUZZLE 3 SOLVED");
            }

            status = PUZZLE_STATUS_SOLVED;
        }
        else
        {
            if (is_dutch())
            {
                print_serial("\r\nFOUTE CODE!\r\n");
                print_serial("Probeer opnieuw.\r\n");
            }
            else
            {
                print_serial("\r\nWRONG CODE!\r\n");
                print_serial("Try again.\r\n");
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

    if (input_index >= MAX_INPUT_LENGTH)
    {
        return;
    }

    user_input[input_index++] = key;
    serial_putchar(key);
    serial_putchar(' ');
}