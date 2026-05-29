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
#include "HAL/BT/hc05.h"
#include "HAL/Storage/logger.h"

volatile uint32_t ms = 0u;

#define HC05_COMMAND_BUFFER_SIZE 32u

static bool time_command_active = false;
static char time_command_buffer[32];
static uint32_t time_command_index = 0u;

static char hc05_command_buffer[HC05_COMMAND_BUFFER_SIZE];
static uint32_t hc05_command_index = 0u;

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

static void hc05_print_current_settings(void)
{
    hc05_write_string("Difficulty: ");
    hc05_write_string(app_settings_difficulty_to_string(app_settings_get_difficulty()));
    hc05_write_string("\r\n");

    hc05_write_string("Language: ");
    hc05_write_string(app_settings_language_to_string(app_settings_get_language()));
    hc05_write_string("\r\n");
}

static int is_digit(char c)
{
    return (c >= '0') && (c <= '9');
}

static int str_equal(const char *a, const char *b)
{
    while ((*a != '\0') && (*b != '\0'))
    {
        char ca = *a;
        char cb = *b;

        if ((ca >= 'a') && (ca <= 'z'))
        {
            ca = (char)(ca - 'a' + 'A');
        }

        if ((cb >= 'a') && (cb <= 'z'))
        {
            cb = (char)(cb - 'a' + 'A');
        }

        if (ca != cb)
        {
            return 0;
        }

        a++;
        b++;
    }

    return (*a == '\0') && (*b == '\0');
}

static void hc05_print_to_usb(const char *s)
{
    print_serial("HC05: ");
    print_serial(s);
    print_serial("\r\n");
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

    while ((*s == ' ') || (*s == '='))
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

    while ((*s == ' ') || (*s == '='))
    {
        s++;
    }

    *year = (uint16_t)parse_fixed_number(s, 4u, &ok);

    if (!ok || (s[4] != '-'))
    {
        return 0;
    }

    *month = (uint8_t)parse_fixed_number(s + 5, 2u, &ok);

    if (!ok || (s[7] != '-'))
    {
        return 0;
    }

    *day = (uint8_t)parse_fixed_number(s + 8, 2u, &ok);

    if (!ok || !((s[10] == ' ') || (s[10] == 'T') || (s[10] == 't')))
    {
        return 0;
    }

    *hour = (uint8_t)parse_fixed_number(s + 11, 2u, &ok);

    if (!ok || (s[13] != ':'))
    {
        return 0;
    }

    *minute = (uint8_t)parse_fixed_number(s + 14, 2u, &ok);

    if (!ok || (s[16] != ':'))
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

    if (ok && (unix_seconds > 1000000000u))
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

    if ((ch == '\r') || (ch == '\n'))
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

static void process_command_char(char ch, bool from_hc05)
{
    app_event_t event;

    event.type = EVENT_NONE;
    event.keypad_key = '\0';
    event.rssi = 0;

    if ((ch == '\r') || (ch == '\n'))
    {
        return;
    }

    logger_log_input_char(ch);

    if (ch == 'E')
    {
        app_settings_set_difficulty(APP_DIFFICULTY_EASY);
        logger_log_settings(from_hc05 ? "HC05 difficulty EASY" : "USB difficulty EASY");

        if (from_hc05)
        {
            hc05_write_string("OK EASY\r\n");
            hc05_print_current_settings();
        }
        else
        {
            print_serial("Difficulty set to EASY\r\n");
            print_current_settings();
        }
    }
    else if (ch == 'H')
    {
        app_settings_set_difficulty(APP_DIFFICULTY_HARD);
        logger_log_settings(from_hc05 ? "HC05 difficulty HARD" : "USB difficulty HARD");

        if (from_hc05)
        {
            hc05_write_string("OK HARD\r\n");
            hc05_print_current_settings();
        }
        else
        {
            print_serial("Difficulty set to HARD\r\n");
            print_current_settings();
        }
    }
    else if (ch == 'P')
    {
        app_settings_set_language(APP_LANGUAGE_ENGLISH);
        logger_log_settings(from_hc05 ? "HC05 language ENGLISH" : "USB language ENGLISH");

        if (from_hc05)
        {
            hc05_write_string("OK ENGLISH\r\n");
            hc05_print_current_settings();
        }
        else
        {
            print_serial("Language set to ENGLISH\r\n");
            print_current_settings();
        }
    }
    else if (ch == 'D')
    {
        app_settings_set_language(APP_LANGUAGE_DUTCH);
        logger_log_settings(from_hc05 ? "HC05 language DUTCH" : "USB language DUTCH");

        if (from_hc05)
        {
            hc05_write_string("OK DUTCH\r\n");
            hc05_print_current_settings();
        }
        else
        {
            print_serial("Language set to DUTCH\r\n");
            print_current_settings();
        }
    }
    else if (ch == 'S')
    {
        logger_log_settings(from_hc05 ? "HC05 settings requested" : "USB settings requested");

        if (from_hc05)
        {
            hc05_write_string("STATUS\r\n");
            hc05_print_current_settings();
        }
        else
        {
            print_current_settings();
        }
    }
    else if (((ch >= '0') && (ch <= '9')) || (ch == '*') || (ch == '#'))
    {
        event.type = EVENT_KEYPAD_KEY;
        event.keypad_key = ch;
    }
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
    else if (ch == 'x')
    {
        logger_log("SYSTEM", from_hc05 ? "HC05 reset requested" : "USB reset requested");
        event.type = EVENT_RESET_REQUEST;
    }
    else
    {
        if (from_hc05)
        {
            hc05_write_string("ERR UNKNOWN\r\n");
        }
        else
        {
            print_serial("Ignored serial input\r\n");
        }

        logger_log("INPUT", "Ignored input");
    }

    send_event(event);
}

static void process_hc05_command(const char *cmd)
{
    app_event_t event;

    event.type = EVENT_NONE;
    event.keypad_key = '\0';
    event.rssi = 0;

    print_serial("HC05 RX: ");
    print_serial(cmd);
    print_serial("\r\n");

    logger_log("HC05", cmd);

    if (str_equal(cmd, "START"))
    {
        /*
         * Currently the MoiBox starts automatically after boot.
         * START is accepted and printed so the app can confirm connection.
         */
        hc05_print_to_usb("CMD START");
        hc05_write_string("OK START\r\n");
    }
    else if (str_equal(cmd, "RESET"))
    {
        hc05_print_to_usb("CMD RESET");
        hc05_write_string("OK RESET\r\n");

        event.type = EVENT_RESET_REQUEST;
    }
    else if (str_equal(cmd, "EASY") || str_equal(cmd, "E"))
    {
        app_settings_set_difficulty(APP_DIFFICULTY_EASY);

        hc05_print_to_usb("CMD EASY");
        hc05_write_string("OK EASY\r\n");
        hc05_print_current_settings();

        print_current_settings();
        logger_log_settings("HC05 difficulty EASY");
    }
    else if (str_equal(cmd, "HARD") || str_equal(cmd, "H"))
    {
        app_settings_set_difficulty(APP_DIFFICULTY_HARD);

        hc05_print_to_usb("CMD HARD");
        hc05_write_string("OK HARD\r\n");
        hc05_print_current_settings();

        print_current_settings();
        logger_log_settings("HC05 difficulty HARD");
    }
    else if (str_equal(cmd, "ENGLISH") || str_equal(cmd, "P"))
    {
        app_settings_set_language(APP_LANGUAGE_ENGLISH);

        hc05_print_to_usb("CMD ENGLISH");
        hc05_write_string("OK ENGLISH\r\n");
        hc05_print_current_settings();

        print_current_settings();
        logger_log_settings("HC05 language ENGLISH");
    }
    else if (str_equal(cmd, "DUTCH") || str_equal(cmd, "D"))
    {
        app_settings_set_language(APP_LANGUAGE_DUTCH);

        hc05_print_to_usb("CMD DUTCH");
        hc05_write_string("OK DUTCH\r\n");
        hc05_print_current_settings();

        print_current_settings();
        logger_log_settings("HC05 language DUTCH");
    }
    else if (str_equal(cmd, "STATUS") || str_equal(cmd, "S"))
    {
        hc05_print_to_usb("CMD STATUS");

        hc05_write_string("STATUS\r\n");
        hc05_print_current_settings();

        print_current_settings();
        logger_log_settings("HC05 status requested");
    }
    else
    {
        /*
         * Keep old single-character commands too:
         * r/g/b/y, u/n/m/j/k, 0-9, *, #, x, etc.
         */
        if ((cmd[0] != '\0') && (cmd[1] == '\0'))
        {
            process_command_char(cmd[0], true);
            return;
        }

        hc05_print_to_usb("UNKNOWN COMMAND");
        hc05_write_string("ERR UNKNOWN\r\n");
    }

    send_event(event);
}

static void handle_serial_input(void)
{
    while (serial_rxcnt() > 0u)
    {
        int ch = serial_getchar();

        if (handle_time_command_char(ch))
        {
            continue;
        }

        process_command_char((char)ch, false);
    }
}

static void handle_hc05_input(void)
{
#if APP_BT_ENABLED
    char ch;

    while (hc05_getchar(&ch))
    {
        if ((ch == '\r') || (ch == '\n'))
        {
            if (hc05_command_index > 0u)
            {
                hc05_command_buffer[hc05_command_index] = '\0';
                process_hc05_command(hc05_command_buffer);
                hc05_command_index = 0u;
            }

            continue;
        }

        if (hc05_command_index < (HC05_COMMAND_BUFFER_SIZE - 1u))
        {
            hc05_command_buffer[hc05_command_index++] = ch;
        }
        else
        {
            hc05_command_index = 0u;
            hc05_write_string("ERR CMD TOO LONG\r\n");
            print_serial("HC05: command too long\r\n");
        }
    }
#endif
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
    handle_hc05_input();

    fsm_update();
}