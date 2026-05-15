#include "leds.h"
#include <MCXA153.h>

// LED configuration
#define LED_GPIO           GPIO3
#define LED_RED_PIN        12U
#define LED_GREEN_PIN      13U
#define LED_BLUE_PIN       0U
#define LED_ACTIVE_LOW     0U
#define LED_INACTIVE_HIGH  1U

// LED state
static bool sw3_mode = false;

// Initialize GPIO for LEDs
void leds_init(void)
{
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_PORT3(1);

    PORT3->PCR[LED_RED_PIN] = PORT_PCR_MUX(0);
    PORT3->PCR[LED_GREEN_PIN] = PORT_PCR_MUX(0);
    PORT3->PCR[LED_BLUE_PIN] = PORT_PCR_MUX(0);

    leds_all_off();
    LED_GPIO->PDDR |= (1U << LED_RED_PIN) | (1U << LED_GREEN_PIN) | (1U << LED_BLUE_PIN);
}

// Control individual LED
static void led_set(uint32_t pin, bool on)
{
    LED_GPIO->PDR[pin] = on ? LED_ACTIVE_LOW : LED_INACTIVE_HIGH;
}

// Set LED color
static void led_set_color(bool red, bool green, bool blue)
{
    leds_all_off();
    if (red)
    {
        led_set(LED_RED_PIN, true);
    }
    if (green)
    {
        led_set(LED_GREEN_PIN, true);
    }
    if (blue)
    {
        led_set(LED_BLUE_PIN, true);
    }
}

// Turn all LEDs off
void leds_all_off(void)
{
    led_set(LED_RED_PIN, false);
    led_set(LED_GREEN_PIN, false);
    led_set(LED_BLUE_PIN, false);
}

// Set LED to red
void leds_set_red(void)
{
    led_set_color(true, false, false);
}

// Set LED to yellow
void leds_set_yellow(void)
{
    led_set_color(true, true, false);
}

//Set LED to blue
void leds_set_blue(bool on)
{
    if (on)
    {
        led_set_color(false, false, true);
    }
    else
    {
        leds_all_off();
    }
}

// Set LED to green
void leds_set_green(void)
{
    led_set_color(false, true, false);
}

// Set LED color based on distance (tx_power)
void leds_set_distance_color(int8_t tx_power)
{
    if (sw3_mode)
    {
        leds_set_blue(true);
        return;
    }

    if (tx_power >= -55)
    {
        leds_set_green();
    }
    else if (tx_power >= -70)
    {
        leds_set_yellow();
    }
    else
    {
        leds_set_red();
    }
}

// Set SW3 mode (blue LED mode)
void leds_set_sw3_mode(bool mode)
{
    sw3_mode = mode;
}

// Get SW3 mode state
bool leds_get_sw3_mode(void)
{
    return sw3_mode;
}
