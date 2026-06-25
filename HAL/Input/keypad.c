#include "HAL/Input/keypad.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * 3x4 keypad layout:
 *
 * 1 2 3
 * 4 5 6
 * 7 8 9
 * * 0 #
 */

static const uint32_t col_pins[KEYPAD_NUM_COLS] =
{
    KEYPAD_COL0_PIN,
    KEYPAD_COL1_PIN,
    KEYPAD_COL2_PIN
};

static const uint32_t row_pins[KEYPAD_NUM_ROWS] =
{
    KEYPAD_ROW0_PIN,
    KEYPAD_ROW1_PIN,
    KEYPAD_ROW2_PIN,
    KEYPAD_ROW3_PIN
};

static const char keypad_map[KEYPAD_NUM_ROWS][KEYPAD_NUM_COLS] =
{
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

static bool keypad_state[KEYPAD_NUM_ROWS][KEYPAD_NUM_COLS] = {0};
static bool keypad_previous[KEYPAD_NUM_ROWS][KEYPAD_NUM_COLS] = {0};

static uint32_t previous_scan_ms = 0u;
static const uint32_t scan_interval_ms = 20u;

static void enable_port2_gpio2_clocks(void)
{
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
}

static void set_all_columns_high(void)
{
    for (uint32_t i = 0u; i < KEYPAD_NUM_COLS; i++)
    {
        KEYPAD_GPIO->PSOR = (1u << col_pins[i]);
    }
}

static void keypad_scan_now(void)
{
    for (uint32_t row = 0u; row < KEYPAD_NUM_ROWS; row++)
    {
        for (uint32_t col = 0u; col < KEYPAD_NUM_COLS; col++)
        {
            keypad_state[row][col] = false;
        }
    }

    /*
     * Matrix scan:
     * - Columns are outputs.
     * - Rows are inputs with pull-ups.
     * - One column is driven LOW at a time.
     * - Pressed key makes row read LOW.
     */
    for (uint32_t col = 0u; col < KEYPAD_NUM_COLS; col++)
    {
        set_all_columns_high();

        KEYPAD_GPIO->PCOR = (1u << col_pins[col]);

        for (volatile uint32_t d = 0u; d < 500u; d++)
        {
            __NOP();
        }

        for (uint32_t row = 0u; row < KEYPAD_NUM_ROWS; row++)
        {
            if ((KEYPAD_GPIO->PDIR & (1u << row_pins[row])) == 0u)
            {
                keypad_state[row][col] = true;
            }
        }
    }

    set_all_columns_high();
}

void keypad_init(void)
{
    enable_port2_gpio2_clocks();

    previous_scan_ms = 0u;

    for (uint32_t row = 0u; row < KEYPAD_NUM_ROWS; row++)
    {
        for (uint32_t col = 0u; col < KEYPAD_NUM_COLS; col++)
        {
            keypad_state[row][col] = false;
            keypad_previous[row][col] = false;
        }
    }

    /*
     * Columns = outputs, idle HIGH.
     */
    for (uint32_t i = 0u; i < KEYPAD_NUM_COLS; i++)
    {
        KEYPAD_PORT->PCR[col_pins[i]] =
            PORT_PCR_MUX(0) |
            PORT_PCR_IBE(1);

        KEYPAD_GPIO->PDDR |= (1u << col_pins[i]);
        KEYPAD_GPIO->PSOR = (1u << col_pins[i]);
    }

    /*
     * Rows = inputs with pull-ups.
     */
    for (uint32_t i = 0u; i < KEYPAD_NUM_ROWS; i++)
    {
        KEYPAD_PORT->PCR[row_pins[i]] =
            PORT_PCR_MUX(0) |
            PORT_PCR_IBE(1) |
            PORT_PCR_PE(1)  |
            PORT_PCR_PS(1);

        KEYPAD_GPIO->PDDR &= ~(1u << row_pins[i]);
    }

    keypad_scan_now();
}

void keypad_update(uint32_t current_ms)
{
    if ((current_ms - previous_scan_ms) < scan_interval_ms)
    {
        return;
    }

    previous_scan_ms = current_ms;

    keypad_scan_now();
}

bool keypad_getkey(char *key)
{
    if (key == 0)
    {
        return false;
    }

    *key = '\0';

    for (uint32_t row = 0u; row < KEYPAD_NUM_ROWS; row++)
    {
        for (uint32_t col = 0u; col < KEYPAD_NUM_COLS; col++)
        {
            if (keypad_state[row][col] && !keypad_previous[row][col])
            {
                keypad_previous[row][col] = true;
                *key = keypad_map[row][col];
                return true;
            }
            
            if (!keypad_state[row][col])
            {
                keypad_previous[row][col] = false;
            }
        }
    }

    return false;
}