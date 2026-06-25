#include <MCXA153.h>

#include "HAL/Input/buttons.h"
#include "App/app.h"

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
 * Behavior:
 * - Holding a button gives only ONE press event.
 * - Another press is only possible after a stable release.
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

#define BUTTON_ACTIVE_LOW  1u
#define BUTTON_DEBOUNCE_MS 50u

typedef struct
{
    uint32_t mask;
    bool raw_pressed;
    bool stable_pressed;
    bool armed;
    bool pending;
    uint32_t last_change_ms;
} button_state_t;

static button_state_t red_button;
static button_state_t green_button;
static button_state_t blue_button;
static button_state_t yellow_button;

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

static void button_state_init(button_state_t *button, uint32_t mask, uint32_t pressed_mask)
{
    button->mask = mask;
    button->raw_pressed = ((pressed_mask & mask) != 0u);
    button->stable_pressed = button->raw_pressed;

    /*
     * If button is held during boot, do not instantly count it.
     * It must be released first.
     */
    button->armed = !button->stable_pressed;

    button->pending = false;
    button->last_change_ms = app_millis();
}

static void button_state_update(button_state_t *button, uint32_t pressed_mask)
{
    bool now_raw_pressed;
    uint32_t now;

    now = app_millis();
    now_raw_pressed = ((pressed_mask & button->mask) != 0u);


    if (now_raw_pressed != button->raw_pressed)
    {
        button->raw_pressed = now_raw_pressed;
        button->last_change_ms = now;
        return;
    }

    if ((uint32_t)(now - button->last_change_ms) < BUTTON_DEBOUNCE_MS)
    {
        return;
    }

    if (button->stable_pressed == button->raw_pressed)
    {
        return;
    }

    button->stable_pressed = button->raw_pressed;

    if (button->stable_pressed)
    {
        if (button->armed)
        {
            button->pending = true;
            button->armed = false;
        }
    }
    else
    {
        button->armed = true;
    }
}

void buttons_init(void)
{
    uint32_t pressed_mask;

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

    pressed_mask = read_pressed_mask();

    button_state_init(&red_button, BUTTON_RED_MASK, pressed_mask);
    button_state_init(&green_button, BUTTON_GREEN_MASK, pressed_mask);
    button_state_init(&blue_button, BUTTON_BLUE_MASK, pressed_mask);
    button_state_init(&yellow_button, BUTTON_YELLOW_MASK, pressed_mask);
}

void buttons_update(void)
{
    uint32_t pressed_mask;

    pressed_mask = read_pressed_mask();

    button_state_update(&red_button, pressed_mask);
    button_state_update(&green_button, pressed_mask);
    button_state_update(&blue_button, pressed_mask);
    button_state_update(&yellow_button, pressed_mask);
}

static bool button_take_pending(button_state_t *button)
{
    if (button->pending)
    {
        button->pending = false;
        return true;
    }

    return false;
}

bool buttons_red_pressed(void)
{
    return button_take_pending(&red_button);
}

bool buttons_green_pressed(void)
{
    return button_take_pending(&green_button);
}

bool buttons_blue_pressed(void)
{
    return button_take_pending(&blue_button);
}

bool buttons_yellow_pressed(void)
{
    return button_take_pending(&yellow_button);
}

bool buttons_is_pressed(void)
{
    return red_button.stable_pressed ||
           green_button.stable_pressed ||
           blue_button.stable_pressed ||
           yellow_button.stable_pressed;
}