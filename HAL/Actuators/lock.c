#include <MCXA153.h>
#include <stdint.h>

#include "HAL/Actuators/lock.h"

/*
 * LOCK CONTROL PIN
 *
 * FRDM-MCXA153 pinout:
 * P3_29 is shown on the Arduino header area.
 *
 * IMPORTANT:
 * This GPIO pin does NOT output 5V.
 * It outputs 3.3V logic HIGH.
 *
 * Use it to control:
 * - relay module IN pin
 * - MOSFET gate
 * - transistor base through resistor
 *
 * External 5V supply powers the lock/solenoid.
 *
 * Example:
 *
 * P3_29 -> relay IN / MOSFET gate
 * Relay/MOSFET switches external 5V to the lock
 * Lock GND and board GND must be connected together
 */

#define LOCK_GPIO GPIO3
#define LOCK_PORT PORT3
#define LOCK_PIN  29u

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
    /*
     * GPIO HIGH.
     * This should activate the relay/MOSFET/transistor.
     * The external circuit then provides 5V to the lock.
     */
    LOCK_GPIO->PSOR = (1u << LOCK_PIN);
}

static void lock_output_off(void)
{
    /*
     * GPIO LOW.
     * Relay/MOSFET/transistor off.
     * Lock power removed.
     */
    LOCK_GPIO->PCOR = (1u << LOCK_PIN);
}

void lock_init(void)
{
    /*
     * Enable PORT3 and GPIO3 clocks.
     *
     * PORT3 is on MRCC_GLB_CC1.
     * GPIO3 is also on MRCC_GLB_CC1.
     */
    MRCC0->MRCC_GLB_CC1_SET =
        MRCC_MRCC_GLB_CC1_PORT3(1) |
        MRCC_MRCC_GLB_CC1_GPIO3(1);

    /*
     * Release PORT3 and GPIO3 from reset.
     */
    MRCC0->MRCC_GLB_RST1_SET =
        MRCC_MRCC_GLB_RST1_PORT3(1) |
        MRCC_MRCC_GLB_RST1_GPIO3(1);

    /*
     * Configure P3_29 as GPIO.
     */
    LOCK_PORT->PCR[LOCK_PIN] = PORT_PCR_MUX(0);

    /*
     * Start OFF before making it output, to avoid a short unwanted pulse.
     */
    lock_output_off();

    /*
     * Set P3_29 as output.
     */
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
     * Turn control signal off.
     * The relay/MOSFET turns off.
     * The lock no longer receives 5V.
     */
    lock_output_off();

    unlocked = 0;
    pulse_active = 0;
}

void lock_unlock(void)
{
    /*
     * Give the relay/MOSFET a HIGH control signal briefly.
     * External 5V is then provided to the lock for 500 ms.
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