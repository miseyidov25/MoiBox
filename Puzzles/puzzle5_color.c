#include "Puzzles/puzzle5_color.h"

#include "App/settings.h"

#include "serial.h"
#include "HAL/Display/oled.h"

#define COLOR_RED     0u
#define COLOR_GREEN   1u
#define COLOR_BLUE    2u
#define COLOR_YELLOW  3u

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;
static uint8_t correct_button = COLOR_BLUE;

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

static void print_button_name(uint8_t button)
{
    if (is_dutch())
    {
        if (button == COLOR_RED)
        {
            print_serial("ROOD");
        }
        else if (button == COLOR_GREEN)
        {
            print_serial("GROEN");
        }
        else if (button == COLOR_BLUE)
        {
            print_serial("BLAUW");
        }
        else if (button == COLOR_YELLOW)
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
}

void puzzle5_color_start(void)
{
    status = PUZZLE_STATUS_RUNNING;

    if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
    {
        correct_button = COLOR_GREEN;
    }
    else
    {
        correct_button = COLOR_BLUE;
    }

    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "PUZZEL 5");

        if (correct_button == COLOR_BLUE)
        {
            oled_display_string(1, 0, "Lucht Zee");
            oled_display_string(2, 0, "Saffier");
        }
        else
        {
            oled_display_string(1, 0, "Gras Blad");
            oled_display_string(2, 0, "Smaragd");
        }

        oled_display_string(3, 0, "Druk kleur");
    }
    else
    {
        oled_display_string(0, 0, "PUZZLE 5");

        if (correct_button == COLOR_BLUE)
        {
            oled_display_string(1, 0, "Sky Ocean");
            oled_display_string(2, 0, "Sapphire");
        }
        else
        {
            oled_display_string(1, 0, "Grass Leaf");
            oled_display_string(2, 0, "Emerald");
        }

        oled_display_string(3, 0, "Press color");
    }

    print_serial("\r\n========== PUZZLE 5: COLOR ==========\r\n");

    print_serial("Difficulty: ");
    print_serial(app_settings_difficulty_to_string(app_settings_get_difficulty()));
    print_serial("\r\n");

    print_serial("Language: ");
    print_serial(app_settings_language_to_string(app_settings_get_language()));
    print_serial("\r\n");

    if (is_dutch())
    {
        print_serial("Lees de woorden en druk de juiste kleur.\r\n");
        print_serial("Antwoord debug: ");
    }
    else
    {
        print_serial("Read clue words and press matching color.\r\n");
        print_serial("Debug answer: ");
    }

    print_button_name(correct_button);
    print_serial("\r\n");
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

    print_serial(is_dutch() ? "Gedrukt: " : "Pressed: ");
    print_button_name(button);
    print_serial("\r\n");

    if (button == correct_button)
    {
        if (is_dutch())
        {
            print_serial("Correcte kleur! Puzzel 5 opgelost.\r\n");

            oled_clear();
            oled_display_string(0, 0, "PUZZEL 5 KLAAR");
        }
        else
        {
            print_serial("Correct color! Puzzle 5 solved.\r\n");

            oled_clear();
            oled_display_string(0, 0, "PUZZLE 5 SOLVED");
        }

        status = PUZZLE_STATUS_SOLVED;
    }
    else
    {
        if (is_dutch())
        {
            print_serial("Foute kleur. Probeer opnieuw.\r\n");

            oled_clear();
            oled_display_string(0, 0, "Foute kleur");
            oled_display_string(1, 0, "Probeer opnieuw");
        }
        else
        {
            print_serial("Wrong color. Try again.\r\n");

            oled_clear();
            oled_display_string(0, 0, "Wrong color");
            oled_display_string(1, 0, "Try again");
        }
    }
}