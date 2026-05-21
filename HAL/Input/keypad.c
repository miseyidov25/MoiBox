#include <MCXA153.h>
#include "HAL/Input/keypad.h"

/*
 * Scan keypad every 20 ms.
 */
static const uint32_t keypad_scan_interval_ms = 20u;

static uint32_t previous_scan_ms = 0u;

/*
 * Current active row.
 */
static uint32_t keypad_row = 0u;

/*
 * Current keypad state.
 */
static bool keypad[4][3] = {0};

/*
 * Previous keypad state.
 * Used to detect new presses only once.
 */
static bool keypad_previous[4][3] = {0};

/*
 * Key mapping table.
 */
static const char keypad_map[4][3] =
{
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

static void keypad_rows_init(void);
static void keypad_columns_init(void);

void keypad_init(void)
{
    keypad_rows_init();
    keypad_columns_init();
}

void keypad_update(uint32_t current_ms)
{
    if ((current_ms - previous_scan_ms) < keypad_scan_interval_ms)
    {
        return;
    }

    previous_scan_ms = current_ms;

    /*
     * Set all rows HIGH first.
     */
    GPIO2->PSOR =
        (1u << 0u) |
        (1u << 1u) |
        (1u << 2u) |
        (1u << 3u);

    /*
     * Drive current row LOW.
     */
    GPIO2->PCOR = (1u << keypad_row);

    /*
     * Small settle delay.
     */
    for (volatile uint32_t d = 0; d < 500u; d++)
    {
        __NOP();
    }

    /*
     * Read columns.
     * Column reads LOW when button is pressed.
     */
    keypad[keypad_row][0] =
        (GPIO1->PDIR & (1u << 8u)) == 0u;

    keypad[keypad_row][1] =
        (GPIO1->PDIR & (1u << 9u)) == 0u;

    keypad[keypad_row][2] =
        (GPIO1->PDIR & (1u << 12u)) == 0u;

    /*
     * Return current row HIGH.
     */
    GPIO2->PSOR = (1u << keypad_row);

    /*
     * Move to next row.
     */
    keypad_row = (keypad_row + 1u) % 4u;
}

bool keypad_getkey(char *key)
{
    if (key == 0)
    {
        return false;
    }

    *key = '\0';

    for (uint32_t row = 0; row < 4u; row++)
    {
        for (uint32_t col = 0; col < 3u; col++)
        {
            /*
             * New key press.
             */
            if (keypad[row][col] && !keypad_previous[row][col])
            {
                keypad_previous[row][col] = true;
                *key = keypad_map[row][col];
                return true;
            }

            /*
             * Key released.
             */
            if (!keypad[row][col])
            {
                keypad_previous[row][col] = false;
            }
        }
    }

    return false;
}

static void keypad_rows_init(void)
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
     * Configure P2_0..P2_3 as GPIO outputs.
     */
    PORT2->PCR[0] = PORT_PCR_MUX(0);
    PORT2->PCR[1] = PORT_PCR_MUX(0);
    PORT2->PCR[2] = PORT_PCR_MUX(0);
    PORT2->PCR[3] = PORT_PCR_MUX(0);

    /*
     * Rows HIGH when idle.
     */
    GPIO2->PSOR =
        (1u << 0u) |
        (1u << 1u) |
        (1u << 2u) |
        (1u << 3u);

    /*
     * Configure rows as outputs.
     */
    GPIO2->PDDR |=
        (1u << 0u) |
        (1u << 1u) |
        (1u << 2u) |
        (1u << 3u);
}

static void keypad_columns_init(void)
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

    /*
     * Configure P1_8, P1_9, P1_12 as GPIO inputs with pull-ups.
     */
    PORT1->PCR[8] =
        PORT_PCR_MUX(0) |
        PORT_PCR_IBE(1) |
        PORT_PCR_PE(1) |
        PORT_PCR_PS(1);

    PORT1->PCR[9] =
        PORT_PCR_MUX(0) |
        PORT_PCR_IBE(1) |
        PORT_PCR_PE(1) |
        PORT_PCR_PS(1);

    PORT1->PCR[12] =
        PORT_PCR_MUX(0) |
        PORT_PCR_IBE(1) |
        PORT_PCR_PE(1) |
        PORT_PCR_PS(1);

    /*
     * Configure columns as inputs.
     */
    GPIO1->PDDR &=
        ~(
            (1u << 8u) |
            (1u << 9u) |
            (1u << 12u)
        );
}