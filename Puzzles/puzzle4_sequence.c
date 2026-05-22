#include "Puzzles/puzzle4_sequence.h"

#include "App/settings.h"

#include "serial.h"
#include "HAL/Display/oled.h"

#define EASY_SEQUENCE_LENGTH 4
#define HARD_SEQUENCE_LENGTH 6

#define BUTTON_RED     0u
#define BUTTON_GREEN   1u
#define BUTTON_BLUE    2u
#define BUTTON_YELLOW  3u

static const uint8_t easy_sequence[EASY_SEQUENCE_LENGTH] =
{
    BUTTON_RED, BUTTON_BLUE, BUTTON_GREEN, BUTTON_YELLOW
};

static const uint8_t hard_sequence[HARD_SEQUENCE_LENGTH] =
{
    BUTTON_RED, BUTTON_BLUE, BUTTON_GREEN, BUTTON_YELLOW, BUTTON_BLUE, BUTTON_RED
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

static int is_dutch(void)
{
    return app_settings_get_language() == APP_LANGUAGE_DUTCH;
}

static uint8_t sequence_length(void)
{
    if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
    {
        return HARD_SEQUENCE_LENGTH;
    }

    return EASY_SEQUENCE_LENGTH;
}

static uint8_t sequence_value(uint8_t index)
{
    if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
    {
        return hard_sequence[index];
    }

    return easy_sequence[index];
}

static void print_button_name(uint8_t button)
{
    if (is_dutch())
    {
        if (button == BUTTON_RED)
        {
            print_serial("ROOD");
        }
        else if (button == BUTTON_GREEN)
        {
            print_serial("GROEN");
        }
        else if (button == BUTTON_BLUE)
        {
            print_serial("BLAUW");
        }
        else if (button == BUTTON_YELLOW)
        {
            print_serial("GEEL");
        }
        else
        {
            print_serial("ONBEKEND");
        }
    }
    else
    {
        if (button == BUTTON_RED)
        {
            print_serial("RED");
        }
        else if (button == BUTTON_GREEN)
        {
            print_serial("GREEN");
        }
        else if (button == BUTTON_BLUE)
        {
            print_serial("BLUE");
        }
        else if (button == BUTTON_YELLOW)
        {
            print_serial("YELLOW");
        }
        else
        {
            print_serial("UNKNOWN");
        }
    }
}

static void show_sequence(void)
{
    if (is_dutch())
    {
        print_serial("Reeks: ");
    }
    else
    {
        print_serial("Sequence: ");
    }

    for (uint8_t i = 0; i < sequence_length(); i++)
    {
        print_button_name(sequence_value(i));

        if (i + 1u < sequence_length())
        {
            print_serial(" ");
        }
    }

    print_serial("\r\n");

    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "PUZZEL 4");
        oled_display_string(1, 0, "Herhaal reeks");
    }
    else
    {
        oled_display_string(0, 0, "PUZZLE 4");
        oled_display_string(1, 0, "Repeat sequence");
    }

    if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
    {
        oled_display_string(2, 0, "R B G Y B R");
    }
    else
    {
        oled_display_string(2, 0, "R B G Y");
    }
}

void puzzle4_sequence_start(void)
{
    status = PUZZLE_STATUS_RUNNING;
    input_index = 0;

    print_serial("\r\n========== PUZZLE 4: SEQUENCE ==========\r\n");

    print_serial("Difficulty: ");
    print_serial(app_settings_difficulty_to_string(app_settings_get_difficulty()));
    print_serial("\r\n");

    print_serial("Language: ");
    print_serial(app_settings_language_to_string(app_settings_get_language()));
    print_serial("\r\n");

    if (is_dutch())
    {
        print_serial("Druk de knoppen in de getoonde volgorde.\r\n");
    }
    else
    {
        print_serial("Press buttons in shown sequence.\r\n");
    }

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

    print_serial(is_dutch() ? "Gedrukt: " : "Pressed: ");
    print_button_name(button);
    print_serial("\r\n");

    if (button == sequence_value(input_index))
    {
        input_index++;

        if (input_index >= sequence_length())
        {
            if (is_dutch())
            {
                print_serial("Correcte reeks! Puzzel 4 opgelost.\r\n");

                oled_clear();
                oled_display_string(0, 0, "PUZZEL 4 KLAAR");
            }
            else
            {
                print_serial("Correct sequence! Puzzle 4 solved.\r\n");

                oled_clear();
                oled_display_string(0, 0, "PUZZLE 4 SOLVED");
            }

            status = PUZZLE_STATUS_SOLVED;
        }
    }
    else
    {
        if (is_dutch())
        {
            print_serial("Foute reeks. Opnieuw.\r\n");

            oled_clear();
            oled_display_string(0, 0, "Fout!");
            oled_display_string(1, 0, "Opnieuw");
        }
        else
        {
            print_serial("Wrong sequence. Restart.\r\n");

            oled_clear();
            oled_display_string(0, 0, "Wrong!");
            oled_display_string(1, 0, "Try again");
        }

        input_index = 0;
        show_sequence();
    }
}