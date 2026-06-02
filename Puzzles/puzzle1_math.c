#include "Puzzles/puzzle1_math.h"

#include "App/settings.h"
#include "App/app.h"

#include "serial.h"
#include "HAL/Display/oled.h"
#include "HAL/Audio/buzzer.h"

#include <MCXA153.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_INPUT_LENGTH 8
#define WRONG_SHOW_MS    1000u

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;

static int apple = 0;
static int banana = 0;
static int first_result = 0;
static int second_result = 0;
static int answer = 0;

static char input[MAX_INPUT_LENGTH + 1];
static int input_index = 0;

static uint32_t random_seed = 1u;

static bool wrong_message_active = false;
static uint32_t wrong_message_until_ms = 0u;

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

static void append_string(char *dst, const char *src, uint32_t max_len)
{
    uint32_t i = 0u;
    uint32_t j = 0u;

    while ((dst[i] != '\0') && (i < max_len))
    {
        i++;
    }

    while ((src[j] != '\0') && (i < max_len))
    {
        dst[i++] = src[j++];
    }

    dst[i] = '\0';
}

static void append_int(char *dst, int value, uint32_t max_len)
{
    char number[12];

    int_to_string(value, number);
    append_string(dst, number, max_len);
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

static int random_range(int min_value, int max_value)
{
    uint32_t range = (uint32_t)(max_value - min_value + 1);
    return min_value + (int)(random_next() % range);
}

static void reset_input(void)
{
    for (int i = 0; i <= MAX_INPUT_LENGTH; i++)
    {
        input[i] = 0;
    }

    input_index = 0;
}

static int input_to_int(void)
{
    int value = 0;

    if (input[0] == '\0')
    {
        return -1;
    }

    for (int i = 0; input[i] != '\0'; i++)
    {
        if ((input[i] < '0') || (input[i] > '9'))
        {
            return -1;
        }

        value = (value * 10) + (input[i] - '0');
    }

    return value;
}

static void generate_problem(void)
{
    random_init();

    apple = random_range(1, 9);
    banana = random_range(1, 9);

    second_result = apple + apple;

    if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
    {
        first_result = banana * apple;
    }
    else
    {
        first_result = banana + apple;
    }

    answer = banana;
}

static void show_problem_oled(void)
{
    char line1[32];
    char line2[32];
    char line3[32];

    line1[0] = '\0';
    line2[0] = '\0';
    line3[0] = '\0';

    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "PUZZEL 1");

        append_string(line1, "BANAAN", 30u);

        if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
        {
            append_string(line1, "xAPPEL=", 30u);
        }
        else
        {
            append_string(line1, "+APPEL=", 30u);
        }

        append_int(line1, first_result, 30u);

        append_string(line2, "APPEL+APPEL=", 30u);
        append_int(line2, second_result, 30u);

        append_string(line3, "BANAAN=", 30u);

        if (input_index > 0)
        {
            append_string(line3, input, 30u);
        }
        else
        {
            append_string(line3, "?", 30u);
        }

        oled_display_string(1, 0, line1);
        oled_display_string(2, 0, line2);
        oled_display_string(3, 0, line3);
    }
    else
    {
        oled_display_string(0, 0, "PUZZLE 1");

        append_string(line1, "BANANA", 30u);

        if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
        {
            append_string(line1, "*APPLE=", 30u);
        }
        else
        {
            append_string(line1, "+APPLE=", 30u);
        }

        append_int(line1, first_result, 30u);

        append_string(line2, "APPLE+APPLE=", 30u);
        append_int(line2, second_result, 30u);

        append_string(line3, "BANANA=", 30u);

        if (input_index > 0)
        {
            append_string(line3, input, 30u);
        }
        else
        {
            append_string(line3, "?", 30u);
        }

        oled_display_string(1, 0, line1);
        oled_display_string(2, 0, line2);
        oled_display_string(3, 0, line3);
    }
}

static void show_wrong_oled(void)
{
    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "FOUT");
        oled_display_string(1, 0, "PROBEER");
        oled_display_string(2, 0, "OPNIEUW");
    }
    else
    {
        oled_display_string(0, 0, "WRONG ANSWER");
        oled_display_string(1, 0, "TRY AGAIN");
    }

    wrong_message_active = true;
    wrong_message_until_ms = app_millis() + WRONG_SHOW_MS;
}

static void print_problem_serial(void)
{
    if (is_dutch())
    {
        if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
        {
            print_serial("Banaan*Appel=");
        }
        else
        {
            print_serial("Banaan+Appel=");
        }

        print_int(first_result);
        print_serial("\r\n");

        print_serial("Appel+Appel=");
        print_int(second_result);
        print_serial("\r\n");

        print_serial("Banaan = ?\r\n");
    }
    else
    {
        if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
        {
            print_serial("Banana*Apple=");
        }
        else
        {
            print_serial("Banana+Apple=");
        }

        print_int(first_result);
        print_serial("\r\n");

        print_serial("Apple+Apple=");
        print_int(second_result);
        print_serial("\r\n");

        print_serial("Banana = ?\r\n");
    }

    print_serial("\r\nDebug values:\r\n");
    print_serial("Apple = ");
    print_int(apple);
    print_serial("\r\n");

    print_serial("Banana = ");
    print_int(banana);
    print_serial("\r\n");

    print_serial("Answer = ");
    print_int(answer);
    print_serial("\r\n\r\n");

    print_serial("# = enter, * = delete\r\n");
    print_serial("Input: ");
}

void puzzle1_math_start(void)
{
    status = PUZZLE_STATUS_RUNNING;
    wrong_message_active = false;
    wrong_message_until_ms = 0u;
    reset_input();

    generate_problem();

    print_serial("\r\n========== PUZZLE 1: MATH ==========\r\n");

    print_serial("Difficulty: ");
    print_serial(app_settings_difficulty_to_string(app_settings_get_difficulty()));
    print_serial("\r\n");

    print_serial("Language: ");
    print_serial(app_settings_language_to_string(app_settings_get_language()));
    print_serial("\r\n");

    show_problem_oled();
    print_problem_serial();
}

puzzle_status_t puzzle1_math_update(void)
{
    if (wrong_message_active && (app_millis() >= wrong_message_until_ms))
    {
        wrong_message_active = false;
        show_problem_oled();
    }

    return status;
}

void puzzle1_math_handle_key(char key)
{
    int user_answer;

    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

    if (wrong_message_active)
    {
        wrong_message_active = false;
        show_problem_oled();
    }

    if (key == '*')
    {
        if (input_index > 0)
        {
            input_index--;
            input[input_index] = 0;

            show_problem_oled();

            if (is_dutch())
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
        print_serial("\r\n");

        user_answer = input_to_int();

        if (user_answer == answer)
        {
            if (is_dutch())
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
            buzzer_error_sound();

            if (is_dutch())
            {
                print_serial("Fout antwoord. Probeer opnieuw.\r\n");
            }
            else
            {
                print_serial("Wrong answer. Try again.\r\n");
            }

            reset_input();
            show_wrong_oled();

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

    input[input_index++] = key;
    input[input_index] = 0;

    serial_putchar(key);

    show_problem_oled();
}