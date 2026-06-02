#include "Puzzles/puzzle3_code.h"

#include "App/settings.h"
#include "App/app.h"

#include "serial.h"
#include "HAL/Display/oled.h"
#include "HAL/Audio/buzzer.h"

#include <stdint.h>
#include <stdbool.h>

#define CODE_LENGTH       7
#define MAX_INPUT_LENGTH  8
#define WRONG_SHOW_MS     1000u

static const char code_digits[CODE_LENGTH + 1] =
{
    '5', '7', '4', '5', '7', '4', '5', '\0'
};

static const char hard_answer[] =
{
    '7', '\0'
};

static char user_input[MAX_INPUT_LENGTH + 1];
static uint8_t input_index = 0u;

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;

static bool wrong_message_active = false;
static uint32_t wrong_message_until_ms = 0u;

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
}

static int is_dutch(void)
{
    return app_settings_get_language() == APP_LANGUAGE_DUTCH;
}

static void reset_input(void)
{
    for (uint8_t i = 0u; i <= MAX_INPUT_LENGTH; i++)
    {
        user_input[i] = '\0';
    }

    input_index = 0u;
}

static int strings_equal(const char *a, const char *b)
{
    while ((*a != '\0') && (*b != '\0'))
    {
        if (*a != *b)
        {
            return 0;
        }

        a++;
        b++;
    }

    return ((*a == '\0') && (*b == '\0'));
}

static void print_code_digits(void)
{
    for (uint8_t i = 0u; i < CODE_LENGTH; i++)
    {
        serial_putchar(code_digits[i]);

        if (i + 1u < CODE_LENGTH)
        {
            print_serial(" ");
        }
    }
}

static void show_easy(void)
{
    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "PUZZEL 3");
        oled_display_string(1, 0, "VOER CODE IN");
        oled_display_string(2, 0, "Input=");
        oled_display_string(3, 0, user_input);
    }
    else
    {
        oled_display_string(0, 0, "PUZZLE 3");
        oled_display_string(1, 0, "ENTER CODE");
        oled_display_string(2, 0, "Input=");
        oled_display_string(3, 0, user_input);
    }
}

static void show_hard(void)
{
    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "PUZZEL 3");
        oled_display_string(1, 0, "VOLGEND GETAL");
        oled_display_string(2, 0, "5 7 4 5 7 4 5 ?");
        oled_display_string(3, 0, "Input=");
    }
    else
    {
        oled_display_string(0, 0, "PUZZLE 3");
        oled_display_string(1, 0, "NEXT NUMBER");
        oled_display_string(2, 0, "5 7 4 5 7 4 5 ?");
        oled_display_string(3, 0, "Input=");
    }

    /*
     * If your OLED line is too short for the full sequence,
     * use the shorter version below instead:
     *
     * oled_display_string(2, 0, "5 7 4 5");
     * oled_display_string(3, 0, "In=");
     */
    oled_display_string(3, 6, user_input);
}

static void show_puzzle_oled(void)
{
    if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
    {
        show_hard();
    }
    else
    {
        show_easy();
    }
}

static void show_wrong_answer(void)
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
        oled_display_string(0, 0, "WRONG");
        oled_display_string(1, 0, "TRY AGAIN");
    }

    wrong_message_active = true;
    wrong_message_until_ms = app_millis() + WRONG_SHOW_MS;
}

static void show_wrong_length(void)
{
    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "FOUTE LENGTE");
        oled_display_string(1, 0, "PROBEER");
        oled_display_string(2, 0, "OPNIEUW");
    }
    else
    {
        oled_display_string(0, 0, "WRONG LENGTH");
        oled_display_string(1, 0, "TRY AGAIN");
    }

    wrong_message_active = true;
    wrong_message_until_ms = app_millis() + WRONG_SHOW_MS;
}

static void print_start_serial(void)
{
    print_serial("\r\n========== PUZZLE 3: CODE ==========\r\n");

    if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
    {
        if (is_dutch())
        {
            print_serial("Zoek het volgende getal in de reeks.\r\n");
        }
        else
        {
            print_serial("Find the next number in the sequence.\r\n");
        }

        print_serial("Sequence: 5 7 4 5 7 4 5 ?\r\n");
        print_serial("Pattern: +2, -3, +1, +2, -3, +1\r\n");
        print_serial("Debug answer: 7\r\n");
    }
    else
    {
        if (is_dutch())
        {
            print_serial("Voer de code van de pagina's in.\r\n");
            print_serial("Pagina nummers: ");
        }
        else
        {
            print_serial("Enter the code from the pages.\r\n");
            print_serial("Page numbers: ");
        }

        print_code_digits();
        print_serial("\r\n");
        print_serial("Code = 5745745\r\n");
    }

    print_serial("# = ENTER, * = DELETE\r\n");
    print_serial("Input: ");
}

static void solved(void)
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

static void wrong(void)
{
    buzzer_error_sound();

    if (is_dutch())
    {
        print_serial("\r\nFOUT!\r\n");
        print_serial("Probeer opnieuw.\r\n");
    }
    else
    {
        print_serial("\r\nWRONG!\r\n");
        print_serial("Try again.\r\n");
    }

    reset_input();
    show_wrong_answer();

    print_serial("Input: ");
}

void puzzle3_code_start(void)
{
    reset_input();
    status = PUZZLE_STATUS_RUNNING;
    wrong_message_active = false;
    wrong_message_until_ms = 0u;

    show_puzzle_oled();
    print_start_serial();
}

puzzle_status_t puzzle3_code_update(void)
{
    if (wrong_message_active && (app_millis() >= wrong_message_until_ms))
    {
        wrong_message_active = false;
        show_puzzle_oled();
    }

    return status;
}

void puzzle3_code_handle_key(char key)
{
    const char *answer;
    uint8_t required_length;

    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

    if (wrong_message_active)
    {
        wrong_message_active = false;
        show_puzzle_oled();
    }

    if (key == '*')
    {
        if (input_index > 0u)
        {
            input_index--;
            user_input[input_index] = '\0';

            show_puzzle_oled();

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
        if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
        {
            answer = hard_answer;
            required_length = 1u;
        }
        else
        {
            answer = code_digits;
            required_length = CODE_LENGTH;
        }

        if (input_index != required_length)
        {
            buzzer_error_sound();

            reset_input();
            show_wrong_length();

            if (is_dutch())
            {
                print_serial("\r\nVerkeerde lengte.\r\nInput: ");
            }
            else
            {
                print_serial("\r\nWrong length.\r\nInput: ");
            }

            return;
        }

        if (strings_equal(user_input, answer))
        {
            solved();
        }
        else
        {
            wrong();
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

    user_input[input_index] = key;
    input_index++;
    user_input[input_index] = '\0';

    serial_putchar(key);
    serial_putchar(' ');

    show_puzzle_oled();
}