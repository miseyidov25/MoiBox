#include "Puzzles/puzzle3_code.h"

#include "App/settings.h"
#include "App/app.h"

#include "serial.h"
#include "HAL/Display/oled.h"

#include <MCXA153.h>
#include <stdint.h>

#define EASY_CODE_LENGTH 7
#define HARD_SEQUENCE_VISIBLE_COUNT 7
#define MAX_INPUT_LENGTH 12

static char easy_code[EASY_CODE_LENGTH + 1];

static int hard_sequence[HARD_SEQUENCE_VISIBLE_COUNT];
static int hard_answer = 0;

static char user_input[MAX_INPUT_LENGTH + 1];
static int input_index = 0;

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;

static uint32_t random_seed = 1u;

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

    while ((value > 0) && (index < 11))
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

    while ((value > 0) && (index < 11))
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

static int is_dutch(void)
{
    return app_settings_get_language() == APP_LANGUAGE_DUTCH;
}

static uint32_t random_next(void)
{
    random_seed = (random_seed * 1103515245u) + 12345u;
    return random_seed;
}

static void random_init(void)
{
    random_seed = app_millis() ^ SysTick->VAL;

    if (random_seed == 0u)
    {
        random_seed = 1u;
    }
}

static int random_range_int(int min_value, int max_value)
{
    uint32_t range = (uint32_t)(max_value - min_value + 1);
    return min_value + (int)(random_next() % range);
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
        if ((*s < '0') || (*s > '9'))
        {
            return -1;
        }

        value = (value * 10) + (*s - '0');
        s++;
    }

    return value;
}

static void generate_easy_code(void)
{
    /*
     * Random 7-digit numeric code.
     *
     * First digit is 1-9 so the code does not start with 0.
     * Other digits can be 0-9.
     */
    easy_code[0] = (char)('1' + (random_next() % 9u));

    for (int i = 1; i < EASY_CODE_LENGTH; i++)
    {
        easy_code[i] = (char)('0' + (random_next() % 10u));
    }

    easy_code[EASY_CODE_LENGTH] = '\0';
}

static void generate_hard_sequence(void)
{
    /*
     * Random arithmetic sequence:
     *
     * Example:
     * start = 3
     * step  = 3
     *
     * 3, 6, 9, 12, 15, 18, 21, ?
     * answer = 24
     */
    int start = random_range_int(2, 12);
    int step = random_range_int(2, 9);

    for (int i = 0; i < HARD_SEQUENCE_VISIBLE_COUNT; i++)
    {
        hard_sequence[i] = start + (step * i);
    }

    hard_answer = start + (step * HARD_SEQUENCE_VISIBLE_COUNT);
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
    for (int i = 0; i < HARD_SEQUENCE_VISIBLE_COUNT; i++)
    {
        print_int(hard_sequence[i]);

        if (i < (HARD_SEQUENCE_VISIBLE_COUNT - 1))
        {
            print_serial(", ");
        }
    }
}

static void show_hard_sequence_oled(void)
{
    char buffer1[32];
    char buffer2[32];
    char temp[12];
    int pos;

    buffer1[0] = '\0';
    buffer2[0] = '\0';

    pos = 0;
    for (int i = 0; i < 4; i++)
    {
        int_to_string(hard_sequence[i], temp);

        int j = 0;
        while ((temp[j] != '\0') && (pos < 30))
        {
            buffer1[pos++] = temp[j++];
        }

        if ((i < 3) && (pos < 30))
        {
            buffer1[pos++] = ' ';
        }
    }

    buffer1[pos] = '\0';

    pos = 0;
    for (int i = 4; i < HARD_SEQUENCE_VISIBLE_COUNT; i++)
    {
        int_to_string(hard_sequence[i], temp);

        int j = 0;
        while ((temp[j] != '\0') && (pos < 30))
        {
            buffer2[pos++] = temp[j++];
        }

        if (pos < 30)
        {
            buffer2[pos++] = ' ';
        }
    }

    if (pos < 30)
    {
        buffer2[pos++] = '?';
    }

    buffer2[pos] = '\0';

    oled_display_string(2, 0, buffer1);
    oled_display_string(3, 0, buffer2);
}

void puzzle3_code_start(void)
{
    reset_input();
    status = PUZZLE_STATUS_RUNNING;

    random_init();

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
        generate_hard_sequence();

        if (is_dutch())
        {
            oled_display_string(0, 0, "PUZZEL 3");
            oled_display_string(1, 0, "Volgend nummer");

            print_serial("Vind het volgende nummer in de reeks.\r\n");
            print_serial("Reeks: ");
            print_hard_sequence();
            print_serial(", ?\r\n");
            print_serial("Voer het volgende nummer in en druk #.\r\n");
        }
        else
        {
            oled_display_string(0, 0, "PUZZLE 3");
            oled_display_string(1, 0, "Next number");

            print_serial("Find the next number in the sequence.\r\n");
            print_serial("Sequence: ");
            print_hard_sequence();
            print_serial(", ?\r\n");
            print_serial("Enter the next number and press #.\r\n");
        }

        show_hard_sequence_oled();

        print_serial("Debug answer: ");
        print_int(hard_answer);
        print_serial("\r\n");
    }
    else
    {
        generate_easy_code();

        if (is_dutch())
        {
            oled_display_string(0, 0, "PUZZEL 3");
            oled_display_string(1, 0, "Voer code in");
        }
        else
        {
            oled_display_string(0, 0, "PUZZLE 3");
            oled_display_string(1, 0, "Enter code");
        }

        print_serial(is_dutch() ? "Voer de code in.\r\n" : "Enter the code.\r\n");

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

    if ((key < '0') || (key > '9'))
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