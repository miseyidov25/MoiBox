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

static volatile uint32_t app_ms = 0u;

void app_tick_1ms(void)
{
    app_ms++;
}

uint32_t app_millis(void)
{
    return app_ms;
}

static void send_event(app_event_t event)
{
    if (event.type != EVENT_NONE)
    {
        fsm_handle_event(event);
    }
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
         * Keypad fallback through serial monitor.
         */
        if ((ch >= '0' && ch <= '9') || ch == '*' || ch == '#')
        {
            event.type = EVENT_KEYPAD_KEY;
            event.keypad_key = (char)ch;
        }

        /*
         * Fake beacon/location events for testing.
         *
         * b = beacon 1
         * n = beacon 2
         * m = beacon 3
         * j = beacon 4
         * k = beacon 5
         */
        else if (ch == 'b')
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
         * Fake colored button events for testing.
         *
         * r = red
         * g = green
         * u = blue
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
        else if (ch == 'u')
        {
            event.type = EVENT_BUTTON_BLUE;
        }
        else if (ch == 'y')
        {
            event.type = EVENT_BUTTON_YELLOW;
        }

        /*
         * Reset test.
         */
        else if (ch == 'x')
        {
            event.type = EVENT_RESET_REQUEST;
        }

        send_event(event);
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

    hal_init();
    fsm_init();

    __enable_irq();
}

void app_update(void)
{
    hal_update(app_millis());

    handle_keypad_input();
    handle_serial_input();
    handle_button_input();
    handle_bt_input();

    fsm_update();
}