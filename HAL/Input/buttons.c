#include <MCXA153.h>

#include "HAL/Input/buttons.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Colored button pin mapping:
 *
 * YELLOW -> P1_6
 * BLUE   -> P1_7
 * GREEN  -> P1_8
 * RED    -> P1_9
 *
 * Buttons are active-low:
 * - Not pressed = HIGH
 * - Pressed     = LOW
 *
 * Each button connects GPIO pin to GND when pressed.
 *
 * Behavior:
 * - Holding a button gives only ONE press event.
 * - To get another press, the button must be released first,
 *   then pressed again.
 */

#define BUTTON_YELLOW_PIN  6u
#define BUTTON_BLUE_PIN    7u
#define BUTTON_GREEN_PIN   8u
#define BUTTON_RED_PIN     9u

#define BUTTON_YELLOW_MASK (1u << BUTTON_YELLOW_PIN)
#define BUTTON_BLUE_MASK   (1u << BUTTON_BLUE_PIN)
#define BUTTON_GREEN_MASK  (1u << BUTTON_GREEN_PIN)
#define BUTTON_RED_MASK    (1u << BUTTON_RED_PIN)

#define BUTTON_ALL_MASK    (BUTTON_YELLOW_MASK | BUTTON_BLUE_MASK | BUTTON_GREEN_MASK | BUTTON_RED_MASK)

#define BUTTON_ACTIVE_LOW  1

/*
 * Number of consecutive identical readings needed before accepting
 * the new button state.
 *
 * If buttons feel too slow, lower this to 3.
 * If buttons still bounce, raise it to 8 or 10.
 */
#define BUTTON_DEBOUNCE_COUNT 5u

static uint32_t last_raw_pressed_mask = 0u;
static uint32_t debounced_pressed_mask = 0u;
static uint8_t stable_count = 0u;

static bool red_pending = false;
static bool green_pending = false;
static bool blue_pending = false;
static bool yellow_pending = false;

static void configure_button_pin(uint32_t pin)
{
    PORT1->PCR[pin] =
        PORT_PCR_MUX(0) |
        PORT_PCR_IBE(1) |
        PORT_PCR_PE(1)  |
        PORT_PCR_PS(1);
}

static uint32_t read_pressed_mask(void)
{
    uint32_t raw;

    raw = GPIO1->PDIR & BUTTON_ALL_MASK;

#if BUTTON_ACTIVE_LOW
    return (~raw) & BUTTON_ALL_MASK;
#else
    return raw & BUTTON_ALL_MASK;
#endif
}

static void queue_new_presses(uint32_t new_press_mask)
{
    if ((new_press_mask & BUTTON_RED_MASK) != 0u)
    {
        red_pending = true;
    }

    if ((new_press_mask & BUTTON_GREEN_MASK) != 0u)
    {
        green_pending = true;
    }

    if ((new_press_mask & BUTTON_BLUE_MASK) != 0u)
    {
        blue_pending = true;
    }

    if ((new_press_mask & BUTTON_YELLOW_MASK) != 0u)
    {
        yellow_pending = true;
    }
}

void buttons_init(void)
{
    /*
     * Enable PORT1 and GPIO1 clocks.
     */
    MRCC0->MRCC_GLB_CC0_SET =
        MRCC_MRCC_GLB_CC0_PORT1(1);

    MRCC0->MRCC_GLB_CC1_SET =
        MRCC_MRCC_GLB_CC1_GPIO1(1);

    /*
     * Release PORT1 and GPIO1 from reset.
     */
    MRCC0->MRCC_GLB_RST0_SET =
        MRCC_MRCC_GLB_RST0_PORT1(1);

    MRCC0->MRCC_GLB_RST1_SET =
        MRCC_MRCC_GLB_RST1_GPIO1(1);

    configure_button_pin(BUTTON_YELLOW_PIN);
    configure_button_pin(BUTTON_BLUE_PIN);
    configure_button_pin(BUTTON_GREEN_PIN);
    configure_button_pin(BUTTON_RED_PIN);

    /*
     * Direction input = 0.
     */
    GPIO1->PDDR &= ~BUTTON_ALL_MASK;

    red_pending = false;
    green_pending = false;
    blue_pending = false;
    yellow_pending = false;

    last_raw_pressed_mask = read_pressed_mask();
    debounced_pressed_mask = last_raw_pressed_mask;
    stable_count = 0u;
}

void buttons_update(void)
{
    uint32_t raw_pressed_mask;
    uint32_t new_press_mask;

    raw_pressed_mask = read_pressed_mask();

    if (raw_pressed_mask == last_raw_pressed_mask)
    {
        if (stable_count < BUTTON_DEBOUNCE_COUNT)
        {
            stable_count++;
        }
    }
    else
    {
        last_raw_pressed_mask = raw_pressed_mask;
        stable_count = 0u;
        return;
    }

    if (stable_count < BUTTON_DEBOUNCE_COUNT)
    {
        return;
    }

    new_press_mask = raw_pressed_mask & (~debounced_pressed_mask);

    debounced_pressed_mask = raw_pressed_mask;

    if (new_press_mask != 0u)
    {
        queue_new_presses(new_press_mask);
    }
}

bool buttons_red_pressed(void)
{
    if (red_pending)
    {
        red_pending = false;
        return true;
    }

    return false;
}

bool buttons_green_pressed(void)
{
    if (green_pending)
    {
        green_pending = false;
        return true;
    }

    return false;
}

bool buttons_blue_pressed(void)
{
    if (blue_pending)
    {
        blue_pending = false;
        return true;
    }

    return false;
}

bool buttons_yellow_pressed(void)
{
    if (yellow_pending)
    {
        yellow_pending = false;
        return true;
    }

    return false;
}

bool buttons_is_pressed(void)
{
    return debounced_pressed_mask != 0u;
}