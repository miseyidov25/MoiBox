#include <MCXA153.h>

#include "HAL/Input/buttons.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Colored button pin mapping from OLED/input branch:
 *
 * YELLOW -> P1_6
 * BLUE   -> P1_7
 * GREEN  -> P1_8
 * RED    -> P1_9
 *
 * Buttons are assumed active-low:
 * - Not pressed = HIGH
 * - Pressed     = LOW
 *
 * Each button should connect GPIO pin to GND when pressed.
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

static uint32_t previous_pressed_mask = 0u;

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

void buttons_init(void)
{
    previous_pressed_mask = 0u;

    red_pending = false;
    green_pending = false;
    blue_pending = false;
    yellow_pending = false;

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
}

void buttons_update(void)
{
    uint32_t pressed_mask;
    uint32_t new_press_mask;

    pressed_mask = read_pressed_mask();
    new_press_mask = pressed_mask & (~previous_pressed_mask);

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

    previous_pressed_mask = pressed_mask;
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
    return red_pending || green_pending || blue_pending || yellow_pending;
}