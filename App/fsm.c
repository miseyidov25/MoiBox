#include "App/fsm.h"
#include "App/events.h"
#include "App/states.h"
#include "App/settings.h"

#include "serial.h"
#include "HAL/Display/oled.h"
#include "HAL/Display/leds.h"
#include "HAL/Actuators/lock.h"
#include "HAL/Storage/logger.h"
#include "Puzzles/puzzle_handler.h"

static app_state_t state = STATE_BOOT;
static puzzle_id_t current_puzzle = PUZZLE_1;
static bool puzzle_solved[PUZZLE_COUNT];

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

static puzzle_id_t beacon_event_to_puzzle(app_event_type_t event_type)
{
    switch (event_type)
    {
        case EVENT_BEACON_1_DETECTED:
            return PUZZLE_1;

        case EVENT_BEACON_2_DETECTED:
            return PUZZLE_2;

        case EVENT_BEACON_3_DETECTED:
            return PUZZLE_3;

        case EVENT_BEACON_4_DETECTED:
            return PUZZLE_4;

        case EVENT_BEACON_5_DETECTED:
            return PUZZLE_5;

        default:
            return PUZZLE_COUNT;
    }
}

static void show_waiting_for_location(void)
{
    oled_clear();

    if (current_puzzle == PUZZLE_1)
    {
        oled_display_string(0, 0, is_dutch() ? "Ga naar plek 1" : "Go to location 1");
        print_serial(is_dutch() ? "\r\nFSM: Wacht op plek 1\r\n" : "\r\nFSM: Waiting for location 1\r\n");
        logger_log("FSM", "Waiting for location 1");
    }
    else if (current_puzzle == PUZZLE_2)
    {
        oled_display_string(0, 0, is_dutch() ? "Ga naar plek 2" : "Go to location 2");
        print_serial(is_dutch() ? "\r\nFSM: Wacht op plek 2\r\n" : "\r\nFSM: Waiting for location 2\r\n");
        logger_log("FSM", "Waiting for location 2");
    }
    else if (current_puzzle == PUZZLE_3)
    {
        oled_display_string(0, 0, is_dutch() ? "Ga naar plek 3" : "Go to location 3");
        print_serial(is_dutch() ? "\r\nFSM: Wacht op plek 3\r\n" : "\r\nFSM: Waiting for location 3\r\n");
        logger_log("FSM", "Waiting for location 3");
    }
    else if (current_puzzle == PUZZLE_4)
    {
        oled_display_string(0, 0, is_dutch() ? "Ga naar plek 4" : "Go to location 4");
        print_serial(is_dutch() ? "\r\nFSM: Wacht op plek 4\r\n" : "\r\nFSM: Waiting for location 4\r\n");
        logger_log("FSM", "Waiting for location 4");
    }
    else if (current_puzzle == PUZZLE_5)
    {
        oled_display_string(0, 0, is_dutch() ? "Ga naar plek 5" : "Go to location 5");
        print_serial(is_dutch() ? "\r\nFSM: Wacht op plek 5\r\n" : "\r\nFSM: Waiting for location 5\r\n");
        logger_log("FSM", "Waiting for location 5");
    }
}

static void handle_puzzle_solved(void)
{
    puzzle_solved[current_puzzle] = true;

    oled_clear();
    oled_display_string(0, 0, is_dutch() ? "Puzzel klaar!" : "Puzzle solved!");

    print_serial(is_dutch() ? "\r\nFSM: Puzzel opgelost\r\n" : "\r\nFSM: Puzzle solved\r\n");
    logger_log("FSM", "Puzzle solved");

    if (current_puzzle == PUZZLE_5)
    {
        state = STATE_ALL_SOLVED;

        oled_clear();
        oled_display_string(0, 0, is_dutch() ? "Alles opgelost" : "All puzzles solved");
        oled_display_string(1, 0, is_dutch() ? "Openen..." : "Unlocking...");

        logger_log("FSM", "All puzzles solved");

        lock_unlock();

        state = STATE_UNLOCKED;

        oled_clear();
        oled_display_string(0, 0, is_dutch() ? "DOOS OPEN" : "BOX UNLOCKED");

        print_serial(is_dutch() ? "\r\nFSM: Doos geopend\r\n" : "\r\nFSM: Box unlocked\r\n");
        logger_log("FSM", "Box unlocked");
    }
    else
    {
        current_puzzle = (puzzle_id_t)(current_puzzle + 1);
        state = STATE_WAIT_FOR_LOCATION;
        show_waiting_for_location();
    }
}

void fsm_init(void)
{
    state = STATE_BOOT;
    current_puzzle = PUZZLE_1;

    for (uint32_t i = 0; i < PUZZLE_COUNT; i++)
    {
        puzzle_solved[i] = false;
    }

    lock_lock();

    oled_clear();
    oled_display_string(0, 0, "MOIBOX START");

    print_serial("\r\nFSM: Boot\r\n");
    logger_log("FSM", "Boot");

    state = STATE_WAIT_FOR_LOCATION;
    show_waiting_for_location();
}

void fsm_update(void)
{
    leds_update_map((uint8_t)current_puzzle, puzzle_solved);

    if (state == STATE_PUZZLE_ACTIVE)
    {
        puzzle_status_t status = puzzle_handler_update(current_puzzle);

        if (status == PUZZLE_STATUS_SOLVED)
        {
            state = STATE_PUZZLE_SOLVED;
            handle_puzzle_solved();
        }
    }
}

void fsm_handle_event(app_event_t event)
{
    if (event.type == EVENT_NONE)
    {
        return;
    }

    if (event.type == EVENT_RESET_REQUEST)
    {
        logger_log("FSM", "Reset");
        state = STATE_RESET;
        fsm_init();
        return;
    }

    if (event.type >= EVENT_BEACON_1_DETECTED &&
        event.type <= EVENT_BEACON_5_DETECTED)
    {
        puzzle_id_t detected = beacon_event_to_puzzle(event.type);

        if (state == STATE_UNLOCKED)
        {
            logger_log("FSM", "Beacon ignored because box already unlocked");
            return;
        }

        if (detected == current_puzzle)
        {
            oled_clear();
            oled_display_string(0, 0, is_dutch() ? "Juiste plek" : "Correct location");

            print_serial(is_dutch() ? "\r\nFSM: Juiste plek gevonden\r\n" : "\r\nFSM: Correct location detected\r\n");
            logger_log("FSM", "Correct location detected");

            puzzle_handler_start(current_puzzle);
            state = STATE_PUZZLE_ACTIVE;
        }
        else
        {
            oled_clear();
            oled_display_string(0, 0, is_dutch() ? "Verkeerde plek" : "Wrong location");
            oled_display_string(1, 0, is_dutch() ? "Ga terug" : "Go back");

            print_serial(is_dutch() ? "\r\nFSM: Verkeerde plek\r\n" : "\r\nFSM: Wrong location detected\r\n");
            logger_log("FSM", "Wrong location detected");

            state = STATE_WRONG_LOCATION;
        }

        return;
    }

    if (event.type == EVENT_KEYPAD_KEY)
    {
        if (state == STATE_PUZZLE_ACTIVE)
        {
            puzzle_handler_handle_key(current_puzzle, event.keypad_key);
        }

        return;
    }

    if (state == STATE_PUZZLE_ACTIVE)
    {
        if (event.type == EVENT_BUTTON_RED)
        {
            puzzle_handler_handle_button(current_puzzle, 0);
        }
        else if (event.type == EVENT_BUTTON_GREEN)
        {
            puzzle_handler_handle_button(current_puzzle, 1);
        }
        else if (event.type == EVENT_BUTTON_BLUE)
        {
            puzzle_handler_handle_button(current_puzzle, 2);
        }
        else if (event.type == EVENT_BUTTON_YELLOW)
        {
            puzzle_handler_handle_button(current_puzzle, 3);
        }
    }
}

app_state_t fsm_get_state(void)
{
    return state;
}

puzzle_id_t fsm_get_current_puzzle(void)
{
    return current_puzzle;
}

bool fsm_is_puzzle_solved(puzzle_id_t puzzle)
{
    if (puzzle >= PUZZLE_COUNT)
    {
        return false;
    }

    return puzzle_solved[puzzle];
}