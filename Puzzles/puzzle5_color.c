#include "Puzzles/puzzle5_color.h"

#include "App/settings.h"
#include "App/app.h"

#include "serial.h"
#include "HAL/Display/oled.h"

#include <MCXA153.h>
#include <stdint.h>

#define COLOR_RED     0u
#define COLOR_GREEN   1u
#define COLOR_BLUE    2u
#define COLOR_YELLOW  3u

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;
static uint8_t correct_button = COLOR_BLUE;
static uint32_t random_seed = 1u;

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
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

static const char *clue_line1_english(uint8_t color)
{
    if (color == COLOR_RED)
    {
        return "Fire Rose";
    }

    if (color == COLOR_GREEN)
    {
        return "Grass Leaf";
    }

    if (color == COLOR_BLUE)
    {
        return "Sky Ocean";
    }

    return "Sun Lemon";
}

static const char *clue_line2_english(uint8_t color)
{
    if (color == COLOR_RED)
    {
        return "Ruby";
    }

    if (color == COLOR_GREEN)
    {
        return "Emerald";
    }

    if (color == COLOR_BLUE)
    {
        return "Sapphire";
    }

    return "Banana";
}

static const char *clue_line1_dutch(uint8_t color)
{
    if (color == COLOR_RED)
    {
        return "Vuur Roos";
    }

    if (color == COLOR_GREEN)
    {
        return "Gras Blad";
    }

    if (color == COLOR_BLUE)
    {
        return "Lucht Zee";
    }

    return "Zon Citroen";
}

static const char *clue_line2_dutch(uint8_t color)
{
    if (color == COLOR_RED)
    {
        return "Robijn";
    }

    if (color == COLOR_GREEN)
    {
        return "Smaragd";
    }

    if (color == COLOR_BLUE)
    {
        return "Saffier";
    }

    return "Banaan";
}

static const char *get_clue_line1(void)
{
    if (is_dutch())
    {
        return clue_line1_dutch(correct_button);
    }

    return clue_line1_english(correct_button);
}

static const char *get_clue_line2(void)
{
    if (is_dutch())
    {
        return clue_line2_dutch(correct_button);
    }

    return clue_line2_english(correct_button);
}

static void show_clue_on_oled(void)
{
    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "PUZZEL 5");
        oled_display_string(1, 0, get_clue_line1());
        oled_display_string(2, 0, get_clue_line2());
        oled_display_string(3, 0, "Druk kleur");
    }
    else
    {
        oled_display_string(0, 0, "PUZZLE 5");
        oled_display_string(1, 0, get_clue_line1());
        oled_display_string(2, 0, get_clue_line2());
        oled_display_string(3, 0, "Press color");
    }
}

void puzzle5_color_start(void)
{
    status = PUZZLE_STATUS_RUNNING;

    random_init();

    /*
     * Random answer:
     * 0 = red
     * 1 = green
     * 2 = blue
     * 3 = yellow
     */
    correct_button = (uint8_t)(random_next() % 4u);

    show_clue_on_oled();

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
        print_serial("Woorden: ");
    }
    else
    {
        print_serial("Read clue words and press matching color.\r\n");
        print_serial("Clue words: ");
    }

    print_serial(get_clue_line1());
    print_serial(" / ");
    print_serial(get_clue_line2());
    print_serial("\r\n");

    print_serial(is_dutch() ? "Antwoord debug: " : "Debug answer: ");
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
        }
        else
        {
            print_serial("Wrong color. Try again.\r\n");
        }

        show_clue_on_oled();
    }
}