#include "HAL/Display/leds.h"

#include <MCXA153.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Location RGB LEDs:
 *
 * RGB1 RED   -> P3_10
 * RGB1 GREEN -> P3_11
 *
 * RGB2 RED   -> P3_13
 * RGB2 GREEN -> P3_14
 *
 * RGB3 RED   -> P2_12
 * RGB3 GREEN -> P2_13
 *
 * RGB4 RED   -> P1_10
 * RGB4 GREEN -> P1_11
 *
 * RGB5 RED   -> P1_13
 * RGB5 GREEN -> P3_0
 *
 * Common GND, active HIGH.
 *
 * Meaning:
 * RED      = not next location and not completed
 * YELLOW   = next location
 * GREEN    = completed location
 * BLINKING = current/target location
 */

typedef struct
{
    GPIO_Type *gpio_red;
    PORT_Type *port_red;
    uint32_t pin_red;

    GPIO_Type *gpio_green;
    PORT_Type *port_green;
    uint32_t pin_green;
} rgb_led_t;

static const rgb_led_t location_leds[5] =
{
    { GPIO3, PORT3, 10u, GPIO3, PORT3, 11u },
    { GPIO3, PORT3, 13u, GPIO3, PORT3, 14u },
    { GPIO2, PORT2, 12u, GPIO2, PORT2, 13u },
    { GPIO1, PORT1, 10u, GPIO1, PORT1, 11u },
    { GPIO1, PORT1, 13u, GPIO3, PORT3, 0u  }
};

static uint32_t last_ms = 0u;
static bool blink_on = true;

static uint8_t active_puzzle_number = 1u;

static uint32_t wrong_flash_until_ms = 0u;
static uint8_t wrong_flash_puzzle_number = 0u;

static bool solved_copy[5] =
{
    false, false, false, false, false
};

static void enable_port_gpio_clocks(void)
{
    /*
     * PORT/GPIO 1
     */
    MRCC0->MRCC_GLB_CC0_SET |= MRCC_MRCC_GLB_CC0_PORT1(1);
    MRCC0->MRCC_GLB_CC1_SET |= MRCC_MRCC_GLB_CC1_GPIO1(1);
    MRCC0->MRCC_GLB_RST0_SET |= MRCC_MRCC_GLB_RST0_PORT1(1);
    MRCC0->MRCC_GLB_RST1_SET |= MRCC_MRCC_GLB_RST1_GPIO1(1);

    /*
     * PORT/GPIO 2
     */
#if defined(MRCC_MRCC_GLB_CC1_PORT2)
    MRCC0->MRCC_GLB_CC1_SET |= MRCC_MRCC_GLB_CC1_PORT2(1);
#elif defined(MRCC_MRCC_GLB_CC0_PORT2)
    MRCC0->MRCC_GLB_CC0_SET |= MRCC_MRCC_GLB_CC0_PORT2(1);
#endif

#if defined(MRCC_MRCC_GLB_CC1_GPIO2)
    MRCC0->MRCC_GLB_CC1_SET |= MRCC_MRCC_GLB_CC1_GPIO2(1);
#elif defined(MRCC_MRCC_GLB_CC0_GPIO2)
    MRCC0->MRCC_GLB_CC0_SET |= MRCC_MRCC_GLB_CC0_GPIO2(1);
#endif

#if defined(MRCC_MRCC_GLB_RST1_PORT2)
    MRCC0->MRCC_GLB_RST1_SET |= MRCC_MRCC_GLB_RST1_PORT2(1);
#elif defined(MRCC_MRCC_GLB_RST0_PORT2)
    MRCC0->MRCC_GLB_RST0_SET |= MRCC_MRCC_GLB_RST0_PORT2(1);
#endif

#if defined(MRCC_MRCC_GLB_RST1_GPIO2)
    MRCC0->MRCC_GLB_RST1_SET |= MRCC_MRCC_GLB_RST1_GPIO2(1);
#elif defined(MRCC_MRCC_GLB_RST0_GPIO2)
    MRCC0->MRCC_GLB_RST0_SET |= MRCC_MRCC_GLB_RST0_GPIO2(1);
#endif

    /*
     * PORT/GPIO 3
     */
    MRCC0->MRCC_GLB_CC1_SET |= MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET |= MRCC_MRCC_GLB_CC1_GPIO3(1);
    MRCC0->MRCC_GLB_RST1_SET |= MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET |= MRCC_MRCC_GLB_RST1_GPIO3(1);
}

static void configure_output(GPIO_Type *gpio, PORT_Type *port, uint32_t pin)
{
    port->PCR[pin] =
        PORT_PCR_MUX(0) |
        PORT_PCR_IBE(1);

    gpio->PDDR |= (1u << pin);
}

static void pin_write(GPIO_Type *gpio, uint32_t pin, bool on)
{
    if (on)
    {
        gpio->PSOR = (1u << pin);
    }
    else
    {
        gpio->PCOR = (1u << pin);
    }
}

static void rgb_set(uint8_t index, bool red, bool green)
{
    if (index >= 5u)
    {
        return;
    }

    pin_write(location_leds[index].gpio_red, location_leds[index].pin_red, red);
    pin_write(location_leds[index].gpio_green, location_leds[index].pin_green, green);
}

static void rgb_red(uint8_t index)
{
    rgb_set(index, true, false);
}

static void rgb_green(uint8_t index)
{
    rgb_set(index, false, true);
}

static void rgb_yellow(uint8_t index)
{
    rgb_set(index, true, true);
}

static void rgb_off(uint8_t index)
{
    rgb_set(index, false, false);
}

static void show_normal_map(void)
{
    uint8_t current_index;

    if ((active_puzzle_number < 1u) || (active_puzzle_number > 5u))
    {
        active_puzzle_number = 1u;
    }

    current_index = active_puzzle_number - 1u;

    for (uint8_t i = 0u; i < 5u; i++)
    {
        if (solved_copy[i])
        {
            rgb_green(i);
        }
        else if (i == current_index)
        {
  
            if (blink_on)
            {
                rgb_yellow(i);
            }
            else
            {
                rgb_off(i);
            }
        }
        else
        {
            rgb_red(i);
        }
    }
}

static void show_wrong_location_flash(void)
{
    uint8_t wrong_index;

    show_normal_map();

    if ((wrong_flash_puzzle_number < 1u) || (wrong_flash_puzzle_number > 5u))
    {
        return;
    }

    wrong_index = wrong_flash_puzzle_number - 1u;

    if (blink_on)
    {
        rgb_red(wrong_index);
    }
    else
    {
        rgb_off(wrong_index);
    }
}

void leds_init(void)
{
    enable_port_gpio_clocks();

    last_ms = 0u;
    blink_on = true;
    active_puzzle_number = 1u;
    wrong_flash_until_ms = 0u;
    wrong_flash_puzzle_number = 0u;

    for (uint8_t i = 0u; i < 5u; i++)
    {
        solved_copy[i] = false;

        configure_output(
            location_leds[i].gpio_red,
            location_leds[i].port_red,
            location_leds[i].pin_red
        );

        configure_output(
            location_leds[i].gpio_green,
            location_leds[i].port_green,
            location_leds[i].pin_green
        );

        rgb_red(i);
    }
}

void leds_update(uint32_t current_ms)
{
    last_ms = current_ms;

    if ((current_ms % 500u) < 250u)
    {
        blink_on = true;
    }
    else
    {
        blink_on = false;
    }

    if ((wrong_flash_until_ms != 0u) && (current_ms < wrong_flash_until_ms))
    {
        show_wrong_location_flash();
        return;
    }

    wrong_flash_until_ms = 0u;
    wrong_flash_puzzle_number = 0u;

    show_normal_map();
}

void leds_update_map(uint8_t current_puzzle, const bool solved[5])
{
    if ((current_puzzle < 1u) || (current_puzzle > 5u))
    {
        current_puzzle = 1u;
    }

    active_puzzle_number = current_puzzle;

    if (solved != 0)
    {
        for (uint8_t i = 0u; i < 5u; i++)
        {
            solved_copy[i] = solved[i];
        }
    }

    if ((wrong_flash_until_ms != 0u) && (last_ms < wrong_flash_until_ms))
    {
        show_wrong_location_flash();
    }
    else
    {
        show_normal_map();
    }
}

void leds_set_active_puzzle(uint8_t puzzle_number)
{
    if ((puzzle_number >= 1u) && (puzzle_number <= 5u))
    {
        active_puzzle_number = puzzle_number;
    }
}

void leds_set_wrong_location_flash(uint8_t puzzle_number)
{
    if ((puzzle_number < 1u) || (puzzle_number > 5u))
    {
        return;
    }

    wrong_flash_puzzle_number = puzzle_number;
    wrong_flash_until_ms = last_ms + 2000u;
}