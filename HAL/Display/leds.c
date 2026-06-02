#include "HAL/Display/leds.h"

#include <MCXA153.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * ============================================================
 * LOCATION RGB MAP LEDS
 * ============================================================
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
 * GREEN    = already solved
 * YELLOW   = go to this current puzzle/location
 * RED      = not solved and not current
 * BLINKING = currently detected/nearby location
 */

/*
 * ============================================================
 * NORMAL COLORED LEDS
 * ============================================================
 *
 * Normal LEDs:
 *
 * Green  -> P3_6
 * Blue   -> P3_7
 * Yellow -> P3_30
 * Red    -> P1_12
 */

#define LOCATION_BLINK_MS 3000u

typedef struct
{
    GPIO_Type *gpio_red;
    PORT_Type *port_red;
    uint32_t pin_red;

    GPIO_Type *gpio_green;
    PORT_Type *port_green;
    uint32_t pin_green;
} rgb_led_t;

typedef struct
{
    GPIO_Type *gpio;
    PORT_Type *port;
    uint32_t pin;
} normal_led_t;

static const rgb_led_t location_leds[5] =
{
    { GPIO3, PORT3, 10u, GPIO3, PORT3, 11u },
    { GPIO3, PORT3, 13u, GPIO3, PORT3, 14u },
    { GPIO2, PORT2, 12u, GPIO2, PORT2, 13u },
    { GPIO1, PORT1, 10u, GPIO1, PORT1, 11u },
    { GPIO1, PORT1, 13u, GPIO3, PORT3, 0u  }
};

static const normal_led_t normal_red_led =
{
    GPIO1, PORT1, 12u
};

static const normal_led_t normal_green_led =
{
    GPIO3, PORT3, 6u
};

static const normal_led_t normal_blue_led =
{
    GPIO3, PORT3, 7u
};

static const normal_led_t normal_yellow_led =
{
    GPIO3, PORT3, 30u
};

static uint32_t last_ms = 0u;
static bool blink_on = true;

static uint8_t active_puzzle_number = 1u;

static uint32_t wrong_flash_until_ms = 0u;
static uint8_t wrong_flash_puzzle_number = 0u;

static uint32_t current_location_blink_until_ms = 0u;
static uint8_t current_location_blink_number = 0u;

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

static void pin_toggle(GPIO_Type *gpio, uint32_t pin)
{
    gpio->PTOR = (1u << pin);
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

static void normal_led_write(const normal_led_t *led, bool on)
{
    if (led == 0)
    {
        return;
    }

    pin_write(led->gpio, led->pin, on);
}

static void normal_led_toggle(const normal_led_t *led)
{
    if (led == 0)
    {
        return;
    }

    pin_toggle(led->gpio, led->pin);
}

static void show_base_color(uint8_t index)
{
    if (index >= 5u)
    {
        return;
    }

    if (solved_copy[index])
    {
        rgb_green(index);
    }
    else if (index == (active_puzzle_number - 1u))
    {
        rgb_yellow(index);
    }
    else
    {
        rgb_red(index);
    }
}

static void show_normal_map(void)
{
    uint8_t blink_index;
    bool blink_active = false;

    if ((active_puzzle_number < 1u) || (active_puzzle_number > 5u))
    {
        active_puzzle_number = 1u;
    }

    if ((current_location_blink_until_ms != 0u) &&
        (last_ms < current_location_blink_until_ms) &&
        (current_location_blink_number >= 1u) &&
        (current_location_blink_number <= 5u))
    {
        blink_active = true;
        blink_index = current_location_blink_number - 1u;
    }
    else
    {
        current_location_blink_until_ms = 0u;
        current_location_blink_number = 0u;
        blink_index = 0u;
    }

    for (uint8_t i = 0u; i < 5u; i++)
    {
        if (blink_active && (i == blink_index))
        {
            if (blink_on)
            {
                show_base_color(i);
            }
            else
            {
                rgb_off(i);
            }
        }
        else
        {
            show_base_color(i);
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
    current_location_blink_until_ms = 0u;
    current_location_blink_number = 0u;

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

    configure_output(normal_red_led.gpio, normal_red_led.port, normal_red_led.pin);
    configure_output(normal_green_led.gpio, normal_green_led.port, normal_green_led.pin);
    configure_output(normal_blue_led.gpio, normal_blue_led.port, normal_blue_led.pin);
    configure_output(normal_yellow_led.gpio, normal_yellow_led.port, normal_yellow_led.pin);

    leds_normal_all_off();
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

void leds_set_current_location_blink(uint8_t puzzle_number)
{
    if ((puzzle_number < 1u) || (puzzle_number > 5u))
    {
        return;
    }

    current_location_blink_number = puzzle_number;
    current_location_blink_until_ms = last_ms + LOCATION_BLINK_MS;
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

/*
 * ============================================================
 * NORMAL COLORED LED API
 * ============================================================
 */

void leds_normal_all_off(void)
{
    leds_normal_red_off();
    leds_normal_green_off();
    leds_normal_blue_off();
    leds_normal_yellow_off();
}

void leds_normal_red_on(void)
{
    normal_led_write(&normal_red_led, true);
}

void leds_normal_green_on(void)
{
    normal_led_write(&normal_green_led, true);
}

void leds_normal_blue_on(void)
{
    normal_led_write(&normal_blue_led, true);
}

void leds_normal_yellow_on(void)
{
    normal_led_write(&normal_yellow_led, true);
}

void leds_normal_red_off(void)
{
    normal_led_write(&normal_red_led, false);
}

void leds_normal_green_off(void)
{
    normal_led_write(&normal_green_led, false);
}

void leds_normal_blue_off(void)
{
    normal_led_write(&normal_blue_led, false);
}

void leds_normal_yellow_off(void)
{
    normal_led_write(&normal_yellow_led, false);
}

void leds_normal_red_toggle(void)
{
    normal_led_toggle(&normal_red_led);
}

void leds_normal_green_toggle(void)
{
    normal_led_toggle(&normal_green_led);
}

void leds_normal_blue_toggle(void)
{
    normal_led_toggle(&normal_blue_led);
}

void leds_normal_yellow_toggle(void)
{
    normal_led_toggle(&normal_yellow_led);
}

void leds_normal_set(uint8_t red, uint8_t green, uint8_t blue, uint8_t yellow)
{
    if (red != 0u)
    {
        leds_normal_red_on();
    }
    else
    {
        leds_normal_red_off();
    }

    if (green != 0u)
    {
        leds_normal_green_on();
    }
    else
    {
        leds_normal_green_off();
    }

    if (blue != 0u)
    {
        leds_normal_blue_on();
    }
    else
    {
        leds_normal_blue_off();
    }

    if (yellow != 0u)
    {
        leds_normal_yellow_on();
    }
    else
    {
        leds_normal_yellow_off();
    }
}