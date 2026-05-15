
#include <MCXA153.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// HAL modules
#include "../HAL/Display/leds.h"
#include "../HAL/Display/oled.h"
#include "../HAL/Input/buttons.h"
#include "../HAL/Input/keypad.h"
#include "../HAL/BT/bt.h"

// Communication modules
#include "../serial/serial.h"
#include "lpuart2.h"

// Application modules
#include "../Puzzles/puzzles_handler.h"

// Global State Management
static bool ibeacon_seen = false;
static int8_t last_tx_power = 0;

// Getter/Setter Functions for External Modules
bool get_ibeacon_seen(void)
{
    return ibeacon_seen;
}

void set_ibeacon_seen(bool seen)
{
    ibeacon_seen = seen;
    if (!seen)
    {
        leds_all_off();
    }
}

int8_t get_last_tx_power(void)
{
    return last_tx_power;
}

void set_last_tx_power(int8_t power)
{
    last_tx_power = power;
    if (!leds_get_sw3_mode())
    {
        leds_set_distance_color(power);
    }
}

bool get_sw3_mode(void)
{
    return leds_get_sw3_mode();
}

// Hardware Connection Info
const char hardware_connection[] =
    "Hardware connection:\r\n"
    "\r\n"
    "     MCU-Link                FRDM-MCXA153                  Device\r\n"
    "    +--------+    +----------------------------------+    +--------\r\n"
    "    |      RX|<---|LPUART0_TX/P0_3 < LPUART2_RX/P1_4 |<---| TXD\r\n"
    "<-->|USB   TX|--->|LPUART0_RX/P0_2 > LPUART2_TX/P1_5 |--->| RXD\r\n"
    "    |        |    |                              3V3 |----| VCC\r\n"
    "    |        |    |                              GND |----| GND\r\n"
    "    +--------+    +----------------------------------+    +--------\r\n";

// Button Callback Handler
static void button_event_handler(button_event_t event)
{
    if (event == BUTTON_EVENT_PRESSED)
    {
        leds_set_sw3_mode(true);
        leds_set_blue(true);
        printf("SW3 pressed: LED set to blue\r\n");
    }
    else if (event == BUTTON_EVENT_RELEASED)
    {
        leds_set_sw3_mode(false);
        if (ibeacon_seen)
        {
            leds_set_distance_color(last_tx_power);
        }
        else
        {
            leds_all_off();
        }
    }
}

// Main Application Loop
static void main_loop(void)
{
    while (1)
    {
        // Poll input devices
        buttons_poll();
        puzzles_handle_keypad();
        puzzles_handle_serial();

        // Process data from LPUART2 (BLE)
        if (lpuart2_rxcnt() > 0)
        {
            uint8_t data = lpuart2_getchar();
            serial_putchar(data);
            bt_process_byte(data);
        }
    }
}

// Main Entry Point
int main(void)
{
    // Initialize Hardware Abstraction Layer
    leds_init();
    buttons_init();
    keypad_init();
    oled_init();

    // Initialize Communication
    serial_init(115200);
    lpuart2_init(9600);

    // Initialize Application Modules
    bt_init();
    puzzles_init();

    // Register Button Callback
    buttons_register_callback(button_event_handler);

    // Print Startup Information
    printf("Serial data forwarder with iBeacon detection\r\n");
    printf("Release build %s %s\r\n", __DATE__, __TIME__);
    printf("\r\n");
    printf(hardware_connection);
    printf("\r\n");
    printf("Puzzle Commands:\r\n");
    printf("  Puzzle1, Puzzle2, Puzzle3, Puzzle4, Puzzle5 - Start specific puzzle\r\n");
    printf("  0-9 - Input numbers for active puzzles\r\n");
    printf("\r\n> ");

    // Start Main Application Loop
    main_loop();

    return 0;
}
