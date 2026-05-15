#include "keypad.h"
#include <stddef.h>

// Keypad character mapping: 4 rows x 3 columns
static const char keypad_map[KEYPAD_NUM_ROWS][KEYPAD_NUM_COLS] = KEYPAD_MAP;

// Column pin array for easy iteration
static const uint32_t col_pins[KEYPAD_NUM_COLS] = {
    KEYPAD_COL0_PIN,
    KEYPAD_COL1_PIN,
    KEYPAD_COL2_PIN
};

// Row pin array for easy iteration
static const uint32_t row_pins[KEYPAD_NUM_ROWS] = {
    KEYPAD_ROW0_PIN,
    KEYPAD_ROW1_PIN,
    KEYPAD_ROW2_PIN,
    KEYPAD_ROW3_PIN
};

// Debounce counter
static volatile uint32_t debounce_count = 0;
#define DEBOUNCE_THRESHOLD 3U

// Previous state for debouncing
static bool last_key_pressed = false;

// Initialize keypad GPIO pins
// Configure column pins as outputs
// Configure row pins as inputs with pull-ups
void keypad_init(void)
{
    // Enable GPIO2 clock and reset
    MRCC0->MRCC_GLB_CC1_SET = MRCC_MRCC_GLB_CC1_GPIO2(1);
    MRCC0->MRCC_GLB_RST1_SET = MRCC_MRCC_GLB_RST1_GPIO2(1);

    // Configure column pins as GPIO outputs
    for (uint32_t i = 0; i < KEYPAD_NUM_COLS; i++)
    {
        KEYPAD_COL_PORT->PCR[col_pins[i]] = PORT_PCR_MUX(0);
        KEYPAD_COL_GPIO->PDDR |= (1U << col_pins[i]);
        KEYPAD_COL_GPIO->PDR[col_pins[i]] = 1U;  // Default to high (inactive)
    }

    // Configure row pins as GPIO inputs with pull-ups
    for (uint32_t i = 0; i < KEYPAD_NUM_ROWS; i++)
    {
        KEYPAD_ROW_PORT->PCR[row_pins[i]] = PORT_PCR_MUX(0) |
                                             PORT_PCR_PE(1) |   // Pull-up enable
                                             PORT_PCR_PS(1) |   // Pull-up select
                                             PORT_PCR_IBE(1);   // Input buffer enable
        KEYPAD_ROW_GPIO->PDDR &= ~(1U << row_pins[i]);  // Configure as input
    }

    last_key_pressed = false;
    debounce_count = 0;
}

// Scan keypad matrix and return pressed key if detected
// \param   key - Pointer to store the detected key character
// \return  true if a key was detected and debounced, false otherwise
bool keypad_getkey(char *key)
{
    if (key == NULL)
    {
        return false;
    }

    *key = '\0';

    // Scan each column
    for (uint32_t col = 0; col < KEYPAD_NUM_COLS; col++)
    {
        // Set this column low, others high
        for (uint32_t i = 0; i < KEYPAD_NUM_COLS; i++)
        {
            KEYPAD_COL_GPIO->PDR[col_pins[i]] = (i == col) ? 0U : 1U;
        }

        // Small delay to allow the signal to settle
        for (volatile uint32_t delay = 0; delay < 100; delay++)
        {
            __NOP();
        }

        // Read each row
        for (uint32_t row = 0; row < KEYPAD_NUM_ROWS; row++)
        {
            uint32_t row_bit = (KEYPAD_ROW_GPIO->PDR[row_pins[row]]) & 1U;

            // Key is pressed when row pin reads low (active low)
            if (row_bit == 0U)
            {
                // Debounce: increment counter while key is pressed
                if (debounce_count < DEBOUNCE_THRESHOLD)
                {
                    debounce_count++;
                }
                else if (!last_key_pressed)
                {
                    // Key is debounced and this is the first detection
                    *key = keypad_map[row][col];
                    last_key_pressed = true;
                    
                    // Set all columns high after detecting key
                    for (uint32_t i = 0; i < KEYPAD_NUM_COLS; i++)
                    {
                        KEYPAD_COL_GPIO->PDR[col_pins[i]] = 1U;
                    }
                    
                    return true;
                }

                return false;
            }
        }
    }

    // No key pressed - reset debounce
    if (debounce_count > 0)
    {
        debounce_count--;
    }
    else
    {
        last_key_pressed = false;
    }

    // Set all columns high when idle
    for (uint32_t i = 0; i < KEYPAD_NUM_COLS; i++)
    {
        KEYPAD_COL_GPIO->PDR[col_pins[i]] = 1U;
    }

    return false;
}