#include "Puzzles/puzzle4_sequence.h"

#include "App/settings.h"
#include "App/app.h"

#include "serial.h"
#include "HAL/Display/oled.h"
#include "HAL/Display/leds.h"
#include "HAL/Audio/buzzer.h"

#include <stdint.h>
#include <stdbool.h>

#define EASY_SEQUENCE_LENGTH 4u
#define HARD_SEQUENCE_LENGTH 6u

#define BUTTON_RED     0u
#define BUTTON_GREEN   1u
#define BUTTON_BLUE    2u
#define BUTTON_YELLOW  3u

#define LED_ON_TIME_MS             1000u
#define SEQUENCE_REPEAT_WAIT_MS    5000u
#define BUTTON_FEEDBACK_MS         300u
#define WRONG_SHOW_MS              1000u

typedef enum
{
    PLAYBACK_LED_ON = 0,
    PLAYBACK_WAIT_REPEAT
} playback_state_t;

static uint8_t sequence[HARD_SEQUENCE_LENGTH];

static uint8_t input_index = 0u;
static puzzle_status_t status = PUZZLE_STATUS_RUNNING;

static playback_state_t playback_state = PLAYBACK_LED_ON;
static uint8_t playback_index = 0u;
static uint32_t next_playback_ms = 0u;

static bool button_feedback_active = false;
static uint32_t button_feedback_until_ms = 0u;

static bool wrong_message_active = false;
static uint32_t wrong_message_until_ms = 0u;

static uint32_t random_seed = 1u;

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

static uint32_t random_next(void)
{
    random_seed = (random_seed * 1103515245u) + 12345u;
    return random_seed;
}

static uint8_t random_button(void)
{
    return (uint8_t)((random_next() >> 16) % 4u);
}

static void generate_sequence(void)
{
    uint8_t length = sequence_length();

    random_seed = app_millis();

    if (random_seed == 0u)
    {
        random_seed = 1u;
    }

    if (app_settings_get_difficulty() == APP_DIFFICULTY_EASY)
    {
        sequence[0] = BUTTON_RED;
        sequence[1] = BUTTON_GREEN;
        sequence[2] = BUTTON_BLUE;
        sequence[3] = BUTTON_YELLOW;

        for (int i = 3; i > 0; i--)
        {
            uint8_t j = (uint8_t)(random_next() % ((uint32_t)i + 1u));
            uint8_t temp = sequence[i];

            sequence[i] = sequence[j];
            sequence[j] = temp;
        }
    }
    else
    {
        for (uint8_t i = 0u; i < length; i++)
        {
            sequence[i] = random_button();
        }
    }
}

static uint8_t sequence_value(uint8_t index)
{
    if (index >= sequence_length())
    {
        return BUTTON_RED;
    }

    return sequence[index];
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

static void normal_led_on_for_button(uint8_t button)
{
    leds_normal_all_off();

    if (button == BUTTON_RED)
    {
        leds_normal_red_on();
    }
    else if (button == BUTTON_GREEN)
    {
        leds_normal_green_on();
    }
    else if (button == BUTTON_BLUE)
    {
        leds_normal_blue_on();
    }
    else if (button == BUTTON_YELLOW)
    {
        leds_normal_yellow_on();
    }
}

static void start_button_feedback(uint8_t button)
{
    normal_led_on_for_button(button);

    button_feedback_active = true;
    button_feedback_until_ms = app_millis() + BUTTON_FEEDBACK_MS;
}

static void print_sequence(void)
{
    if (is_dutch())
    {
        print_serial("Reeks: ");
    }
    else
    {
        print_serial("Sequence: ");
    }

    for (uint8_t i = 0u; i < sequence_length(); i++)
    {
        print_button_name(sequence_value(i));

        if (i + 1u < sequence_length())
        {
            print_serial(" ");
        }
    }

    print_serial("\r\n");
}

static void show_oled_info(void)
{
    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "PUZZEL 4");
        oled_display_string(1, 0, "Kijk naar LEDs");
        oled_display_string(2, 0, "Herhaal reeks");
        oled_display_string(3, 0, "Wacht = replay");
    }
    else
    {
        oled_display_string(0, 0, "PUZZLE 4");
        oled_display_string(1, 0, "Watch LEDs");
        oled_display_string(2, 0, "Repeat sequence");
        oled_display_string(3, 0, "Wait = replay");
    }
}

static void show_wrong_oled(void)
{
    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "FOUT!");
        oled_display_string(1, 0, "OPNIEUW");
    }
    else
    {
        oled_display_string(0, 0, "WRONG!");
        oled_display_string(1, 0, "TRY AGAIN");
    }

    wrong_message_active = true;
    wrong_message_until_ms = app_millis() + WRONG_SHOW_MS;
}

static void playback_restart(void)
{
    playback_state = PLAYBACK_LED_ON;
    playback_index = 0u;
    next_playback_ms = app_millis();

    button_feedback_active = false;
    button_feedback_until_ms = 0u;

    leds_normal_all_off();
}

static void playback_update(void)
{
    uint32_t now;

    if (status == PUZZLE_STATUS_SOLVED)
    {
        leds_normal_all_off();
        return;
    }

    now = app_millis();

    if (wrong_message_active)
    {
        if (now >= wrong_message_until_ms)
        {
            wrong_message_active = false;
            show_oled_info();
            playback_restart();
        }

        return;
    }

    if (button_feedback_active)
    {
        if (now >= button_feedback_until_ms)
        {
            button_feedback_active = false;
            leds_normal_all_off();
            next_playback_ms = now + 200u;
        }

        return;
    }

    if (now < next_playback_ms)
    {
        return;
    }

    if (playback_state == PLAYBACK_LED_ON)
    {
        if (playback_index < sequence_length())
        {
            normal_led_on_for_button(sequence_value(playback_index));

            playback_index++;
            next_playback_ms = now + LED_ON_TIME_MS;
        }
        else
        {
            leds_normal_all_off();

            playback_state = PLAYBACK_WAIT_REPEAT;
            next_playback_ms = now + SEQUENCE_REPEAT_WAIT_MS;
        }
    }
    else
    {
        playback_index = 0u;
        playback_state = PLAYBACK_LED_ON;
        next_playback_ms = now;
    }
}

void puzzle4_sequence_start(void)
{
    status = PUZZLE_STATUS_RUNNING;
    input_index = 0u;
    wrong_message_active = false;
    wrong_message_until_ms = 0u;

    generate_sequence();
    playback_restart();

    print_serial("\r\n========== PUZZLE 4: SEQUENCE ==========\r\n");

    print_serial("Difficulty: ");
    print_serial(app_settings_difficulty_to_string(app_settings_get_difficulty()));
    print_serial("\r\n");

    print_serial("Language: ");
    print_serial(app_settings_language_to_string(app_settings_get_language()));
    print_serial("\r\n");

    if (is_dutch())
    {
        print_serial("Kijk naar de LEDs en druk de knoppen in dezelfde volgorde.\r\n");
        print_serial("De reeks wordt elke 5 seconden herhaald.\r\n");
    }
    else
    {
        print_serial("Watch the LEDs and press the buttons in the same order.\r\n");
        print_serial("The sequence repeats every 5 seconds.\r\n");
    }

    print_sequence();
    show_oled_info();
}

puzzle_status_t puzzle4_sequence_update(void)
{
    playback_update();
    return status;
}

void puzzle4_sequence_handle_button(uint8_t button)
{
    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

    if (wrong_message_active)
    {
        return;
    }

    print_serial(is_dutch() ? "Gedrukt: " : "Pressed: ");
    print_button_name(button);
    print_serial("\r\n");

    start_button_feedback(button);

    if (button == sequence_value(input_index))
    {
        input_index++;

        if (input_index >= sequence_length())
        {
            leds_normal_all_off();

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
        buzzer_error_sound();

        if (is_dutch())
        {
            print_serial("Foute reeks. Opnieuw.\r\n");
        }
        else
        {
            print_serial("Wrong sequence. Restart.\r\n");
        }

        input_index = 0u;
        button_feedback_active = false;
        leds_normal_all_off();

        show_wrong_oled();
    }
}