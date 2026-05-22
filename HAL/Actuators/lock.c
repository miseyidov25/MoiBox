#include <MCXA153.h>
#include <stdint.h>

#include "HAL/Actuators/lock.h"

/*
 * LOCK CONTROL PIN
 *
 * This pin does NOT power the lock directly.
 * It controls a MOSFET/transistor/relay module.
 *
 * Example:
 * GPIO P2_10 -> MOSFET gate / relay IN
 */
#define LOCK_GPIO GPIO2
#define LOCK_PORT PORT2
#define LOCK_PIN  10u

/*
 * Lock opens when powered.
 * Keep power on only briefly.
 */
#define LOCK_UNLOCK_PULSE_MS 500u

static int unlocked = 0;
static int pulse_active = 0;

static uint32_t last_ms = 0u;
static uint32_t pulse_end_ms = 0u;

static void lock_output_on(void)
{
    LOCK_GPIO->PSOR = (1u << LOCK_PIN);
}

static void lock_output_off(void)
{
    LOCK_GPIO->PCOR = (1u << LOCK_PIN);
}

void lock_init(void)
{
    /*
     * Enable PORT2 and GPIO2 clocks.
     */
    MRCC0->MRCC_GLB_CC0_SET =
        MRCC_MRCC_GLB_CC0_PORT2(1);

    MRCC0->MRCC_GLB_CC1_SET =
        MRCC_MRCC_GLB_CC1_GPIO2(1);

    /*
     * Release PORT2 and GPIO2 from reset.
     */
    MRCC0->MRCC_GLB_RST0_SET =
        MRCC_MRCC_GLB_RST0_PORT2(1);

    MRCC0->MRCC_GLB_RST1_SET =
        MRCC_MRCC_GLB_RST1_GPIO2(1);

    /*
     * Configure lock control pin as GPIO output.
     */
    LOCK_PORT->PCR[LOCK_PIN] = PORT_PCR_MUX(0);

    lock_output_off();

    LOCK_GPIO->PDDR |= (1u << LOCK_PIN);

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

void lock_lock(void)
{
    /*
     * Turn power off.
     * The lock physically locks mechanically.
     */
    lock_output_off();

    unlocked = 0;
    pulse_active = 0;
}

void lock_unlock(void)
{
    /*
     * Give lock power briefly.
     */
    lock_output_on();

    unlocked = 1;
    pulse_active = 1;
    pulse_end_ms = last_ms + LOCK_UNLOCK_PULSE_MS;
}

int lock_is_unlocked(void)
{
    return unlocked;
}