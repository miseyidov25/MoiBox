#include <stdint.h>
#include <stdbool.h>

#include "Puzzles/puzzle2_morsecode.h"

#include "App/settings.h"
#include "App/app.h"

#include "serial.h"
#include "HAL/Display/oled.h"
#include "HAL/Audio/buzzer.h"

#include <MCXA153.h>

#define NUMBER_CODE_LENGTH 3u
#define MORSE_TEXT_MAX     32u

#define COLOR_RED     0u
#define COLOR_GREEN   1u
#define COLOR_BLUE    2u
#define COLOR_YELLOW  3u

#define WRONG_SHOW_MS       1000u
#define AUTO_REPLAY_MS      5000u

static puzzle_status_t status = PUZZLE_STATUS_RUNNING;
static uint8_t clue_played = 0u;

static char morse_text[MORSE_TEXT_MAX];

static char number_code[NUMBER_CODE_LENGTH + 1u];

static char input[NUMBER_CODE_LENGTH + 1u];
static uint8_t input_index = 0u;

static uint8_t correct_color = COLOR_RED;
static uint32_t random_seed = 1u;

static bool wrong_message_active = false;
static uint32_t wrong_message_until_ms = 0u;
static uint32_t next_auto_replay_ms = 0u;

static const char *color_words_english[] =
{
    "RED",
    "GREEN",
    "BLUE",
    "YELLOW"
};

static const char *color_words_dutch[] =
{
    "ROOD",
    "GROEN",
    "BLAUW",
    "GEEL"
};

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

static void generate_random_color(void)
{
    correct_color = (uint8_t)(random_next() % 4u);
}

static void generate_random_number_code(void)
{
    number_code[0] = (char)('1' + (random_next() % 9u));
    number_code[1] = (char)('0' + (random_next() % 10u));
    number_code[2] = (char)('0' + (random_next() % 10u));
    number_code[3] = '\0';
}

static const char *morse_for_char(char c)
{
    if ((c >= 'a') && (c <= 'z'))
    {
        c = (char)(c - 'a' + 'A');
    }

    switch (c)
    {
        case 'A': return ".-";
        case 'B': return "-...";
        case 'C': return "-.-.";
        case 'D': return "-..";
        case 'E': return ".";
        case 'F': return "..-.";
        case 'G': return "--.";
        case 'H': return "....";
        case 'I': return "..";
        case 'J': return ".---";
        case 'K': return "-.-";
        case 'L': return ".-..";
        case 'M': return "--";
        case 'N': return "-.";
        case 'O': return "---";
        case 'P': return ".--.";
        case 'Q': return "--.-";
        case 'R': return ".-.";
        case 'S': return "...";
        case 'T': return "-";
        case 'U': return "..-";
        case 'V': return "...-";
        case 'W': return ".--";
        case 'X': return "-..-";
        case 'Y': return "-.--";
        case 'Z': return "--..";

        case '0': return "-----";
        case '1': return ".----";
        case '2': return "..---";
        case '3': return "...--";
        case '4': return "....-";
        case '5': return ".....";
        case '6': return "-....";
        case '7': return "--...";
        case '8': return "---..";
        case '9': return "----.";

        default: return "";
    }
}

static void append_char_to_morse(char c)
{
    uint8_t i = 0u;

    while ((morse_text[i] != '\0') && (i < (MORSE_TEXT_MAX - 1u)))
    {
        i++;
    }

    if (i < (MORSE_TEXT_MAX - 1u))
    {
        morse_text[i] = c;
        morse_text[i + 1u] = '\0';
    }
}

static void append_string_to_morse(const char *s)
{
    while (*s)
    {
        append_char_to_morse(*s);
        s++;
    }
}

static void make_morse_text(const char *text)
{
    morse_text[0] = '\0';

    while (*text)
    {
        append_string_to_morse(morse_for_char(*text));

        text++;

        if (*text)
        {
            append_char_to_morse(' ');
        }
    }
}

static const char *get_color_word(void)
{
    if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
    {
        return color_words_dutch[correct_color];
    }

    return color_words_english[correct_color];
}

static void reset_input(void)
{
    for (uint8_t i = 0u; i <= NUMBER_CODE_LENGTH; i++)
    {
        input[i] = 0;
    }

    input_index = 0u;
}

static int number_answer_correct(void)
{
    for (uint8_t i = 0u; i < NUMBER_CODE_LENGTH; i++)
    {
        if (input[i] != number_code[i])
        {
            return 0;
        }
    }

    return 1;
}

static void set_next_replay(void)
{
    next_auto_replay_ms = app_millis() + AUTO_REPLAY_MS;
}

static void show_current_clue(void)
{
    if (app_settings_get_difficulty() == APP_DIFFICULTY_EASY)
    {
        const char *word = get_color_word();

        make_morse_text(word);

        oled_clear();

        if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
        {
            oled_display_string(0, 0, "PUZZEL 2");
            oled_display_string(1, 0, "Morse kleur");
            oled_display_string(2, 0, "Luister goed");
            oled_display_string(3, 0, "Druk kleur");
        }
        else
        {
            oled_display_string(0, 0, "PUZZLE 2");
            oled_display_string(1, 0, "Morse color");
            oled_display_string(2, 0, "Listen carefully");
            oled_display_string(3, 0, "Press color");
        }

        print_serial("Morse color clue: ");
        print_serial(word);
        print_serial("\r\n");

        print_serial("Dots/dashes: ");
        print_serial(morse_text);
        print_serial("\r\n");
    }
    else
    {
        make_morse_text(number_code);

        oled_clear();

        if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
        {
            oled_display_string(0, 0, "PUZZEL 2");
            oled_display_string(1, 0, "Morse nummer");
            oled_display_string(2, 0, "Luister goed");
            oled_display_string(3, 0, "Voer in + #");
        }
        else
        {
            oled_display_string(0, 0, "PUZZLE 2");
            oled_display_string(1, 0, "Morse number");
            oled_display_string(2, 0, "Listen carefully");
            oled_display_string(3, 0, "Enter + #");
        }

        print_serial("Morse number clue: ");
        print_serial(number_code);
        print_serial("\r\n");

        print_serial("Dots/dashes: ");
        print_serial(morse_text);
        print_serial("\r\n");
    }
}

static void show_wrong_color(void)
{
    oled_clear();

    if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
    {
        oled_display_string(0, 0, "FOUTE KLEUR");
        oled_display_string(1, 0, "PROBEER");
        oled_display_string(2, 0, "OPNIEUW");
    }
    else
    {
        oled_display_string(0, 0, "WRONG COLOR");
        oled_display_string(1, 0, "TRY AGAIN");
    }

    wrong_message_active = true;
    wrong_message_until_ms = app_millis() + WRONG_SHOW_MS;
}

static void show_wrong_number(void)
{
    oled_clear();

    if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
    {
        oled_display_string(0, 0, "VERKEERD");
        oled_display_string(1, 0, "NUMMER");
        oled_display_string(2, 0, "OPNIEUW");
    }
    else
    {
        oled_display_string(0, 0, "WRONG NUMBER");
        oled_display_string(1, 0, "TRY AGAIN");
    }

    wrong_message_active = true;
    wrong_message_until_ms = app_millis() + WRONG_SHOW_MS;
}

static void show_wrong_length(void)
{
    oled_clear();

    if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
    {
        oled_display_string(0, 0, "3 CIJFERS");
        oled_display_string(1, 0, "PROBEER");
        oled_display_string(2, 0, "OPNIEUW");
    }
    else
    {
        oled_display_string(0, 0, "ENTER 3 DIGITS");
        oled_display_string(1, 0, "TRY AGAIN");
    }

    wrong_message_active = true;
    wrong_message_until_ms = app_millis() + WRONG_SHOW_MS;
}

static void play_current_clue(void)
{
    if (app_settings_get_difficulty() == APP_DIFFICULTY_EASY)
    {
        buzzer_morse_string(get_color_word());
    }
    else
    {
        buzzer_morse_string(number_code);
    }

    set_next_replay();
}

void puzzle2_morsecode_start(void)
{
    status = PUZZLE_STATUS_RUNNING;
    clue_played = 0u;
    wrong_message_active = false;
    wrong_message_until_ms = 0u;
    next_auto_replay_ms = 0u;

    reset_input();

    random_init();
    generate_random_color();
    generate_random_number_code();

    print_serial("\r\n========== PUZZLE 2: MORSE ==========\r\n");

    print_serial("Difficulty: ");
    print_serial(app_settings_difficulty_to_string(app_settings_get_difficulty()));
    print_serial("\r\n");

    print_serial("Language: ");
    print_serial(app_settings_language_to_string(app_settings_get_language()));
    print_serial("\r\n");

    if (app_settings_get_difficulty() == APP_DIFFICULTY_EASY)
    {
        if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
        {
            print_serial("Luister naar Morse en druk de juiste kleur.\r\n");
            print_serial("Druk # om opnieuw af te spelen.\r\n");
        }
        else
        {
            print_serial("Listen to Morse and press matching color button.\r\n");
            print_serial("Press # to replay.\r\n");
        }
    }
    else
    {
        if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
        {
            print_serial("Luister naar Morse, voer 3 cijfers in, druk #.\r\n");
            print_serial("Druk # met lege invoer om opnieuw af te spelen.\r\n");
            print_serial("Input: ");
        }
        else
        {
            print_serial("Listen to Morse, enter 3 digits, press #.\r\n");
            print_serial("Press # with empty input to replay.\r\n");
            print_serial("Input: ");
        }
    }

    show_current_clue();
}

puzzle_status_t puzzle2_morsecode_update(void)
{
    if (status == PUZZLE_STATUS_SOLVED)
    {
        return status;
    }

    if (wrong_message_active && (app_millis() >= wrong_message_until_ms))
    {
        wrong_message_active = false;
        show_current_clue();
        set_next_replay();
    }

    if (!clue_played)
    {
        clue_played = 1u;
        play_current_clue();
    }
    else if (!wrong_message_active &&
             (next_auto_replay_ms != 0u) &&
             (app_millis() >= next_auto_replay_ms))
    {
        play_current_clue();
    }

    return status;
}

void puzzle2_morsecode_handle_key(char key)
{
    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

    if (wrong_message_active)
    {
        wrong_message_active = false;
        show_current_clue();
    }

    set_next_replay();

    if (app_settings_get_difficulty() == APP_DIFFICULTY_EASY)
    {
        if (key == '#')
        {
            show_current_clue();
            play_current_clue();
        }

        return;
    }

    if (key == '#')
    {
        if (input_index == 0u)
        {
            show_current_clue();
            play_current_clue();
            print_serial("Input: ");
            return;
        }

        if (input_index != NUMBER_CODE_LENGTH)
        {
            buzzer_error_sound();
            show_wrong_length();

            if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
            {
                print_serial("\r\nVoer precies 3 cijfers in.\r\nInput: ");
            }
            else
            {
                print_serial("\r\nEnter exactly 3 digits.\r\nInput: ");
            }

            reset_input();
            return;
        }

        print_serial("\r\n");

        if (number_answer_correct())
        {
            if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
            {
                print_serial("Correct Morse nummer! Puzzel 2 opgelost.\r\n");

                oled_clear();
                oled_display_string(0, 0, "PUZZEL 2 KLAAR");
            }
            else
            {
                print_serial("Correct Morse number! Puzzle 2 solved.\r\n");

                oled_clear();
                oled_display_string(0, 0, "PUZZLE 2 SOLVED");
            }

            status = PUZZLE_STATUS_SOLVED;
        }
        else
        {
            buzzer_error_sound();
            show_wrong_number();

            if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
            {
                print_serial("Verkeerd nummer. Probeer opnieuw.\r\n");
            }
            else
            {
                print_serial("Wrong number. Try again.\r\n");
            }

            reset_input();
            print_serial("Input: ");
        }

        return;
    }

    if (key == '*')
    {
        if (input_index > 0u)
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

    if ((key < '0') || (key > '9'))
    {
        return;
    }

    if (input_index >= NUMBER_CODE_LENGTH)
    {
        return;
    }

    input[input_index] = key;
    input_index++;

    serial_putchar(key);
    serial_putchar(' ');
}

void puzzle2_morsecode_handle_button(uint8_t button)
{
    if (status == PUZZLE_STATUS_SOLVED)
    {
        return;
    }

    if (app_settings_get_difficulty() == APP_DIFFICULTY_HARD)
    {
        (void)button;
        return;
    }

    if (wrong_message_active)
    {
        wrong_message_active = false;
        show_current_clue();
    }

    set_next_replay();

    if (button == correct_color)
    {
        if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
        {
            print_serial("Correcte kleur! Puzzel 2 opgelost.\r\n");

            oled_clear();
            oled_display_string(0, 0, "PUZZEL 2 KLAAR");
        }
        else
        {
            print_serial("Correct color! Puzzle 2 solved.\r\n");

            oled_clear();
            oled_display_string(0, 0, "PUZZLE 2 SOLVED");
        }

        status = PUZZLE_STATUS_SOLVED;
    }
    else
    {
        buzzer_error_sound();
        show_wrong_color();

        if (app_settings_get_language() == APP_LANGUAGE_DUTCH)
        {
            print_serial("Foute kleur. Probeer opnieuw.\r\n");
        }
        else
        {
            print_serial("Wrong color. Try again.\r\n");
        }
    }
}