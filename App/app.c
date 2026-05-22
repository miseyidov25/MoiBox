#include "App/app.h"

#include <MCXA153.h>

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

static void print_serial(const char *s)
{
    while (*s)
    {
        serial_putchar(*s++);
    }
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

static void print_rx_char(int ch)
{
    print_serial("\r\nRX: ");

    if (ch == '\r')
    {
        print_serial("\\r");
    }
    else if (ch == '\n')
    {
        print_serial("\\n");
    }
    else
    {
        serial_putchar((char)ch);
    }

    print_serial("\r\n");
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

static void handle_serial_input(void)
{
    app_event_t event;

    while (serial_rxcnt() > 0u)
    {
        int ch = serial_getchar();

        event.type = EVENT_NONE;
        event.keypad_key = '\0';
        event.rssi = 0;

        print_rx_char(ch);
        logger_log_input_char((char)ch);

        if (ch == '\r' || ch == '\n')
        {
            continue;
        }

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
        else if ((ch >= '0' && ch <= '9') || ch == '*' || ch == '#')
        {
            event.type = EVENT_KEYPAD_KEY;
            event.keypad_key = (char)ch;
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

    SysTick_Config(48000u);
    __enable_irq();

    app_settings_init();

    hal_init();

    logger_init();
    logger_log("SYSTEM", "MOIBOX boot");

    fsm_init();

    print_current_settings();
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