#include "App/fsm.h"
#include "App/events.h"
#include "App/states.h"
#include "App/settings.h"

#include "serial.h"

#include "HAL/Display/oled.h"
#include "HAL/Display/leds.h"
#include "HAL/Actuators/lock.h"
#include "HAL/Audio/buzzer.h"
#include "HAL/BT/bt.h"
#include "HAL/Storage/logger.h"

#include "Puzzles/puzzle_handler.h"

static app_state_t state = STATE_BOOT;

/*
 * Current route location, 1..5.
 * This is separate from the puzzle ID.
 */
static uint8_t current_location = 1u;

/*
 * The puzzle currently assigned to the current location.
 */
static puzzle_id_t current_puzzle = PUZZLE_1;

/*
 * This table makes puzzle assignment configurable.
 *
 * Default:
 * Location 1 -> Puzzle 1
 * Location 2 -> Puzzle 2
 * Location 3 -> Puzzle 3
 * Location 4 -> Puzzle 4
 * Location 5 -> Puzzle 5
 *
 * Later the app can change it, for example:
 * fsm_set_puzzle_for_location(1, PUZZLE_5);
 */
static puzzle_id_t puzzle_for_location[5] =
{
    PUZZLE_1,
    PUZZLE_2,
    PUZZLE_3,
    PUZZLE_4,
    PUZZLE_5
};

/*
 * Completed locations are used for map LEDs.
 */
static bool location_completed[5] =
{
    false, false, false, false, false
};

/*
 * Solved puzzles are still tracked separately.
 */
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

static void sync_current_puzzle_from_location(void)
{
    if ((current_location < 1u) || (current_location > 5u))
    {
        current_location = 1u;
    }

    current_puzzle = puzzle_for_location[current_location - 1u];
}

static uint8_t beacon_event_to_location(app_event_type_t event_type)
{
    switch (event_type)
    {
        case EVENT_BEACON_1_DETECTED:
            return 1u;

        case EVENT_BEACON_2_DETECTED:
            return 2u;

        case EVENT_BEACON_3_DETECTED:
            return 3u;

        case EVENT_BEACON_4_DETECTED:
            return 4u;

        case EVENT_BEACON_5_DETECTED:
            return 5u;

        default:
            return 0u;
    }
}

static void print_uint(uint32_t value)
{
    char buffer[12];
    int index = 0;

    if (value == 0u)
    {
        serial_putchar('0');
        return;
    }

    while ((value > 0u) && (index < 11))
    {
        buffer[index++] = (char)('0' + (value % 10u));
        value /= 10u;
    }

    while (index > 0)
    {
        serial_putchar(buffer[--index]);
    }
}

static void oled_show_look_for_current_location(void)
{
    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "ZOEK LOCATIE");
    }
    else
    {
        oled_display_string(0, 0, "LOOK FOR LOC");
    }

    oled_display_value(1, 0, current_location);
}

static void show_waiting_for_location(void)
{
    sync_current_puzzle_from_location();

    state = STATE_WAIT_FOR_LOCATION;

    bt_set_scanning_enabled(true);

    leds_set_active_puzzle(current_location);
    leds_update_map(current_location, location_completed);

    oled_show_look_for_current_location();

    if (is_dutch())
    {
        print_serial("\r\nFSM: Zoek locatie ");
    }
    else
    {
        print_serial("\r\nFSM: Looking for location ");
    }

    print_uint(current_location);
    print_serial("\r\n");

    logger_log("FSM", "Waiting for next location");
}

static void show_wrong_location(uint8_t detected_location)
{
    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "VERKEERDE PLEK");
        oled_display_string(1, 0, "JE BENT BIJ");
        oled_display_string(2, 0, "LOCATIE");
        oled_display_value(2, 8, detected_location);
        oled_display_string(3, 0, "ZOEK LOC");
        oled_display_value(3, 8, current_location);
    }
    else
    {
        oled_display_string(0, 0, "WRONG LOCATION");
        oled_display_string(1, 0, "YOU ARE AT");
        oled_display_string(2, 0, "LOCATION");
        oled_display_value(2, 9, detected_location);
        oled_display_string(3, 0, "LOOK FOR");
        oled_display_value(3, 9, current_location);
    }

    print_serial("\r\nFSM: Wrong location reached. Detected location ");
    print_uint(detected_location);
    print_serial(", look for location ");
    print_uint(current_location);
    print_serial(".\r\n");

    logger_log("FSM", "Wrong location reached");

    leds_set_wrong_location_flash(detected_location);
    buzzer_error_sound();

    /*
     * Stay in searching mode.
     * BT scanning remains enabled.
     */
    state = STATE_WAIT_FOR_LOCATION;
}

static void start_current_puzzle(void)
{
    sync_current_puzzle_from_location();

    state = STATE_PUZZLE_ACTIVE;

    bt_set_scanning_enabled(false);

    leds_set_active_puzzle(current_location);
    leds_update_map(current_location, location_completed);

    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "JUISTE PLEK");
    }
    else
    {
        oled_display_string(0, 0, "CORRECT LOC");
    }

    print_serial("\r\nFSM: Correct location detected. Starting location ");
    print_uint(current_location);
    print_serial(", puzzle ");
    print_uint(((uint8_t)current_puzzle) + 1u);
    print_serial(".\r\n");

    logger_log("FSM", "Correct location detected");

    puzzle_handler_start(current_puzzle);
}

static void handle_puzzle_solved(void)
{
    location_completed[current_location - 1u] = true;

    if (current_puzzle < PUZZLE_COUNT)
    {
        puzzle_solved[current_puzzle] = true;
    }

    buzzer_correct_sound();

    oled_clear();

    if (is_dutch())
    {
        oled_display_string(0, 0, "PUZZEL KLAAR");
    }
    else
    {
        oled_display_string(0, 0, "PUZZLE SOLVED");
    }

    print_serial("\r\nFSM: Location ");
    print_uint(current_location);
    print_serial(" solved. Puzzle ");
    print_uint(((uint8_t)current_puzzle) + 1u);
    print_serial(" completed.\r\n");

    logger_log("FSM", "Puzzle solved");

    if (current_location >= 5u)
    {
        state = STATE_ALL_SOLVED;

        bt_set_scanning_enabled(false);

        leds_update_map(5u, location_completed);

        oled_clear();

        if (is_dutch())
        {
            oled_display_string(0, 0, "ALLES KLAAR");
            oled_display_string(1, 0, "DOOS OPEN");
        }
        else
        {
            oled_display_string(0, 0, "ALL SOLVED");
            oled_display_string(1, 0, "BOX OPEN");
        }

        lock_unlock();

        state = STATE_UNLOCKED;

        print_serial("\r\nFSM: Box unlocked\r\n");
        logger_log("FSM", "Box unlocked");
        logger_end_run();

        return;
    }

    current_location++;
    sync_current_puzzle_from_location();

    show_waiting_for_location();
}

void fsm_init(void)
{
    state = STATE_BOOT;

    current_location = 1u;
    sync_current_puzzle_from_location();

    for (uint32_t i = 0u; i < PUZZLE_COUNT; i++)
    {
        puzzle_solved[i] = false;
    }

    for (uint32_t i = 0u; i < 5u; i++)
    {
        location_completed[i] = false;
    }

    bt_set_scanning_enabled(false);

    lock_lock();

    oled_clear();
    oled_display_string(0, 0, "MOIBOX");

    print_serial("\r\nFSM: Boot\r\n");
    logger_log("FSM", "Boot");

    show_waiting_for_location();
}

void fsm_update(void)
{
    leds_update_map(current_location, location_completed);

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
    uint8_t detected_location;

    if (event.type == EVENT_NONE)
    {
        return;
    }

    if (event.type == EVENT_RESET_REQUEST)
    {
        logger_log("FSM", "Reset requested");
        logger_end_run();
        fsm_init();
        logger_start_run();
        return;
}

    if ((event.type >= EVENT_BEACON_1_DETECTED) &&
        (event.type <= EVENT_BEACON_5_DETECTED))
    {
        detected_location = beacon_event_to_location(event.type);

        if (state == STATE_UNLOCKED)
        {
            logger_log("FSM", "Beacon ignored because box is unlocked");
            return;
        }

        if (state == STATE_PUZZLE_ACTIVE)
        {
            logger_log("FSM", "Beacon ignored because puzzle is active");
            return;
        }

        if (detected_location == current_location)
        {
            start_current_puzzle();
        }
        else
        {
            show_wrong_location(detected_location);
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

uint8_t fsm_get_current_location(void)
{
    return current_location;
}

bool fsm_is_puzzle_solved(puzzle_id_t puzzle)
{
    if (puzzle >= PUZZLE_COUNT)
    {
        return false;
    }

    return puzzle_solved[puzzle];
}

void fsm_set_puzzle_for_location(uint8_t location_number, puzzle_id_t puzzle)
{
    if ((location_number < 1u) || (location_number > 5u))
    {
        return;
    }

    if (puzzle >= PUZZLE_COUNT)
    {
        return;
    }

    puzzle_for_location[location_number - 1u] = puzzle;

    if (location_number == current_location)
    {
        sync_current_puzzle_from_location();
    }
}

puzzle_id_t fsm_get_puzzle_for_location(uint8_t location_number)
{
    if ((location_number < 1u) || (location_number > 5u))
    {
        return PUZZLE_COUNT;
    }

    return puzzle_for_location[location_number - 1u];
}