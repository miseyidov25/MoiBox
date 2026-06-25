#include <MCXA153.h>
#include <stdint.h>

#include "HAL/Actuators/lock.h"

/*
 * LOCK CONTROL PIN
 *
 * P3_29 controls the relay/MOSFET/transistor input.
 *
 * IMPORTANT:
 * The GPIO does NOT power the lock directly.
 * The lock must get external 5V through relay/MOSFET/transistor.
 *
 * If your relay module opens the lock immediately with the old code,
 * it is probably ACTIVE LOW.
 *
 * This file uses ACTIVE LOW logic:
 *
 * P3_29 HIGH = relay/MOSFET OFF = lock has no 5V = locked
 * P3_29 LOW  = relay/MOSFET ON  = lock gets 5V = opens
 */

#define LOCK_GPIO GPIO3
#define LOCK_PORT PORT3
#define LOCK_PIN  29u

#define LOCK_UNLOCK_PULSE_MS 1000u

#define LOCK_ACTIVE_LOW 0u

static int unlocked = 0;
static int pulse_active = 0;

static uint32_t last_ms = 0u;
static uint32_t pulse_end_ms = 0u;

static void lock_pin_high(void)
{
    LOCK_GPIO->PSOR = (1u << LOCK_PIN);
}

static void lock_pin_low(void)
{
    LOCK_GPIO->PCOR = (1u << LOCK_PIN);
}

static void lock_output_on(void)
{
#if LOCK_ACTIVE_LOW
    lock_pin_low();
#else
    lock_pin_high();
#endif
}

static void lock_output_off(void)
{
#if LOCK_ACTIVE_LOW
    lock_pin_high();
#else
    lock_pin_low();
#endif
}

void lock_init(void)
{
    MRCC0->MRCC_GLB_CC1_SET =
        MRCC_MRCC_GLB_CC1_PORT3(1) |
        MRCC_MRCC_GLB_CC1_GPIO3(1);

    MRCC0->MRCC_GLB_RST1_SET =
        MRCC_MRCC_GLB_RST1_PORT3(1) |
        MRCC_MRCC_GLB_RST1_GPIO3(1);

    LOCK_PORT->PCR[LOCK_PIN] = PORT_PCR_MUX(0);

    lock_output_off();

    LOCK_GPIO->PDDR |= (1u << LOCK_PIN);

    lock_output_off();

    unlocked = 0;
    pulse_active = 0;
    last_ms = 0u;
    pulse_end_ms = 0u;
}

void lock_update(uint32_t current_ms)
{
    last_ms = current_ms;

    if (pulse_active)
    {
        if ((int32_t)(current_ms - pulse_end_ms) >= 0)
        {
            lock_output_off();
            pulse_active = 0;
        }
    }
}

void lock_unlock(void)
{
    lock_output_on();

    unlocked = 1;
    pulse_active = 1;
    pulse_end_ms = last_ms + LOCK_UNLOCK_PULSE_MS;
}

void lock_lock(void)
{
    lock_output_off();

    unlocked = 0;
    pulse_active = 0;
}

int lock_is_unlocked(void)
{
    return unlocked;
}