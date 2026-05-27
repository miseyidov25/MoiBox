#include "App/app.h"

#include <MCXA153.h>
#include <stdint.h>
#include <stdbool.h>

#include "App/events.h"
#include "App/fsm.h"
#include "App/settings.h"

#include "serial.h"

#include "HAL/hal.h"
#include "HAL/Input/keypad.h"
#include "HAL/Input/buttons.h"
#include "HAL/BT/bt.h"
#include "HAL/Storage/logger.h"

volatile uint32_t ms = 0u;

static bool time_command_active = false;
static char time_command_buffer[32];
static uint32_t time_command_index = 0u;

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
}

void SysTick_Handler(void)
{
    app_tick_1ms();
}

void app_tick_1ms(void)
{
    ms++;
}

uint32_t app_millis(void)
{
    return ms;
}

static void send_event(app_event_t event)
{
    if (event.type != EVENT_NONE)
    {
        logger_log_event(event_to_string(event.type));
        fsm_handle_event(event);
    }
}

static void print_current_settings(void)
{
    print_serial("Difficulty: ");
    print_serial(app_settings_difficulty_to_string(app_settings_get_difficulty()));
    print_serial("\r\n");

    print_serial("Language: ");
    print_serial(app_settings_language_to_string(app_settings_get_language()));
    print_serial("\r\n");
}

static int is_digit(char c)
{
    return (c >= '0') && (c <= '9');
}

static uint32_t parse_uint32(const char *s, int *ok)
{
    uint32_t value = 0u;
    int found_digit = 0;

    if (ok != 0)
    {
        *ok = 0;
    }

    if (s == 0)
    {
        return 0u;
    }

    while (*s == ' ' || *s == '=')
    {
        s++;
    }

    while (is_digit(*s))
    {
        found_digit = 1;
        value = (value * 10u) + (uint32_t)(*s - '0');
        s++;
    }

    if (ok != 0)
    {
        *ok = found_digit;
    }

    return value;
}

static uint32_t parse_fixed_number(const char *s, uint32_t count, int *ok)
{
    uint32_t value = 0u;

    if (ok != 0)
    {
        *ok = 0;
    }

    if (s == 0)
    {
        return 0u;
    }

    for (uint32_t i = 0u; i < count; i++)
    {
        if (!is_digit(s[i]))
        {
            return 0u;
        }

        value = (value * 10u) + (uint32_t)(s[i] - '0');
    }

    if (ok != 0)
    {
        *ok = 1;
    }

    return value;
}

static int parse_datetime_command(
    const char *s,
    uint16_t *year,
    uint8_t *month,
    uint8_t *day,
    uint8_t *hour,
    uint8_t *minute,
    uint8_t *second
)
{
    int ok;

    if ((s == 0) || (year == 0) || (month == 0) || (day == 0) ||
        (hour == 0) || (minute == 0) || (second == 0))
    {
        return 0;
    }

    while (*s == ' ' || *s == '=')
    {
        s++;
    }

    /*
     * Format:
     * T2026-05-27 14:30:00
     * T2026-05-27T14:30:00
     */
    *year = (uint16_t)parse_fixed_number(s, 4u, &ok);

    if (!ok || s[4] != '-')
    {
        return 0;
    }

    *month = (uint8_t)parse_fixed_number(s + 5, 2u, &ok);

    if (!ok || s[7] != '-')
    {
        return 0;
    }

    *day = (uint8_t)parse_fixed_number(s + 8, 2u, &ok);

    if (!ok || !((s[10] == ' ') || (s[10] == 'T') || (s[10] == 't')))
    {
        return 0;
    }

    *hour = (uint8_t)parse_fixed_number(s + 11, 2u, &ok);

    if (!ok || s[13] != ':')
    {
        return 0;
    }

    *minute = (uint8_t)parse_fixed_number(s + 14, 2u, &ok);

    if (!ok || s[16] != ':')
    {
        return 0;
    }

    *second = (uint8_t)parse_fixed_number(s + 17, 2u, &ok);

    if (!ok)
    {
        return 0;
    }

    if ((*month < 1u) || (*month > 12u) ||
        (*day < 1u) || (*day > 31u) ||
        (*hour > 23u) ||
        (*minute > 59u) ||
        (*second > 59u))
    {
        return 0;
    }

    return 1;
}

static void process_time_command(void)
{
    const char *s = time_command_buffer;
    int ok;
    uint32_t unix_seconds;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    time_command_buffer[time_command_index] = '\0';

    if ((s[0] == 'U') || (s[0] == 'u'))
    {
        s++;
    }

    unix_seconds = parse_uint32(s, &ok);

    if (ok && unix_seconds > 1000000000u)
    {
        logger_set_unix_time(unix_seconds, app_millis());
        print_serial("TIME: synchronized from Unix seconds\r\n");
        return;
    }

    if (parse_datetime_command(
            time_command_buffer,
            &year,
            &month,
            &day,
            &hour,
            &minute,
            &second))
    {
        logger_set_datetime(year, month, day, hour, minute, second, app_millis());
        print_serial("TIME: synchronized from date/time\r\n");
        return;
    }

    print_serial("TIME: invalid format\r\n");
    print_serial("Use: T2026-05-27 14:30:00\r\n");
    print_serial("Or:  TU1716810000\r\n");
}

static int handle_time_command_char(int ch)
{
    if (!time_command_active)
    {
        if (ch == 'T')
        {
            time_command_active = true;
            time_command_index = 0u;
            return 1;
        }

        return 0;
    }

    if (ch == '\r' || ch == '\n')
    {
        process_time_command();

        time_command_active = false;
        time_command_index = 0u;

        return 1;
    }

    if (time_command_index < (sizeof(time_command_buffer) - 1u))
    {
        time_command_buffer[time_command_index++] = (char)ch;
    }

    return 1;
}

static void handle_serial_input(void)
{
    app_event_t event;

    while (serial_rxcnt() > 0u)
    {
        int ch = serial_getchar();

        event.type = EVENT_NONE;
        event.keypad_key = '\0';
        event.rssi = 0;

        /*
         * Time sync command.
         *
         * Send from PC/app:
         * T2026-05-27 14:30:00
         *
         * Or Unix seconds:
         * TU1716810000
         */
        if (handle_time_command_char(ch))
        {
            continue;
        }

        /*
         * Ignore line endings completely.
         * This removes RX: \r, RX: \n, INPUT: \r, INPUT: \n.
         */
        if (ch == '\r' || ch == '\n')
        {
            continue;
        }

        logger_log_input_char((char)ch);

        /*
         * Runtime settings:
         *
         * E = easy difficulty
         * H = hard difficulty
         * P = English language
         * D = Dutch language
         * S = show current settings
         */
        if (ch == 'E')
        {
            app_settings_set_difficulty(APP_DIFFICULTY_EASY);
            print_serial("Difficulty set to EASY\r\n");
            logger_log_settings("Difficulty set to EASY");
            print_current_settings();
        }
        else if (ch == 'H')
        {
            app_settings_set_difficulty(APP_DIFFICULTY_HARD);
            print_serial("Difficulty set to HARD\r\n");
            logger_log_settings("Difficulty set to HARD");
            print_current_settings();
        }
        else if (ch == 'P')
        {
            app_settings_set_language(APP_LANGUAGE_ENGLISH);
            print_serial("Language set to ENGLISH\r\n");
            logger_log_settings("Language set to ENGLISH");
            print_current_settings();
        }
        else if (ch == 'D')
        {
            app_settings_set_language(APP_LANGUAGE_DUTCH);
            print_serial("Language set to DUTCH\r\n");
            logger_log_settings("Language set to DUTCH");
            print_current_settings();
        }
        else if (ch == 'S')
        {
            print_current_settings();
            logger_log_settings("Settings printed");
        }

        /*
         * Keypad fallback through serial:
         * 0-9, *, #
         */
        else if ((ch >= '0' && ch <= '9') || ch == '*' || ch == '#')
        {
            event.type = EVENT_KEYPAD_KEY;
            event.keypad_key = (char)ch;
        }

        /*
         * Fake beacon/location events:
         *
         * u = location 1
         * n = location 2
         * m = location 3
         * j = location 4
         * k = location 5
         */
        else if (ch == 'u')
        {
            event.type = EVENT_BEACON_1_DETECTED;
        }
        else if (ch == 'n')
        {
            event.type = EVENT_BEACON_2_DETECTED;
        }
        else if (ch == 'm')
        {
            event.type = EVENT_BEACON_3_DETECTED;
        }
        else if (ch == 'j')
        {
            event.type = EVENT_BEACON_4_DETECTED;
        }
        else if (ch == 'k')
        {
            event.type = EVENT_BEACON_5_DETECTED;
        }

        /*
         * Fake colored button events:
         *
         * r = red
         * g = green
         * b = blue
         * y = yellow
         */
        else if (ch == 'r')
        {
            event.type = EVENT_BUTTON_RED;
        }
        else if (ch == 'g')
        {
            event.type = EVENT_BUTTON_GREEN;
        }
        else if (ch == 'b')
        {
            event.type = EVENT_BUTTON_BLUE;
        }
        else if (ch == 'y')
        {
            event.type = EVENT_BUTTON_YELLOW;
        }

        /*
         * Reset:
         * x = reset
         */
        else if (ch == 'x')
        {
            logger_log("SYSTEM", "Reset requested");
            event.type = EVENT_RESET_REQUEST;
        }
        else
        {
            print_serial("Ignored serial input\r\n");
            logger_log("INPUT", "Ignored serial input");
        }

        if (event.type != EVENT_NONE)
        {
            send_event(event);
        }
    }
}

static void handle_keypad_input(void)
{
#if APP_KEYPAD_ENABLED
    char key;
    app_event_t event;

    if (keypad_getkey(&key))
    {
        event.type = EVENT_KEYPAD_KEY;
        event.keypad_key = key;
        event.rssi = 0;

        logger_log_input_char(key);
        send_event(event);
    }
#endif
}

static void handle_button_input(void)
{
#if APP_BUTTONS_ENABLED
    app_event_t event;

    event.keypad_key = '\0';
    event.rssi = 0;

    if (buttons_red_pressed())
    {
        event.type = EVENT_BUTTON_RED;
        send_event(event);
    }

    if (buttons_green_pressed())
    {
        event.type = EVENT_BUTTON_GREEN;
        send_event(event);
    }

    if (buttons_blue_pressed())
    {
        event.type = EVENT_BUTTON_BLUE;
        send_event(event);
    }

    if (buttons_yellow_pressed())
    {
        event.type = EVENT_BUTTON_YELLOW;
        send_event(event);
    }
#endif
}

static void handle_bt_input(void)
{
#if APP_BT_ENABLED
    app_event_t event;

    if (bt_get_event(&event))
    {
        send_event(event);
    }
#endif
}

void app_init(void)
{
    serial_init(115200);

    /*
     * 48 MHz clock.
     * 48000 cycles = 1 ms.
     */
    SysTick_Config(48000u);
    __enable_irq();

    print_serial("\r\nAPP: boot\r\n");

    app_settings_init();

    hal_init();

    /*
     * Start the puzzle box before logger_init().
     * This prevents bad/missing SD from stopping startup messages.
     */
    fsm_init();

    print_current_settings();

    logger_init();

    if (logger_is_available())
    {
        print_serial("LOGGER: initialized\r\n");
    }
    else
    {
        print_serial("LOGGER: unavailable\r\n");
    }

    logger_log("SYSTEM", "MOIBOX boot");
    logger_start_run();
}

void app_update(void)
{
    logger_update(app_millis());

    hal_update(app_millis());

    handle_keypad_input();
    handle_serial_input();
    handle_button_input();
    handle_bt_input();

    fsm_update();
}