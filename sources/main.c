/*
 * PUZZLE BOX - FRDM-MCXA153 Main Application
 * ===========================================
 * 
 * Target Device: FRDM-MCXA153 (NXP MCX A153 Arm Cortex-M33)
 * Framework: MCUXpresso SDK
 * 
 * Hardware Peripheral Overview:
 * ┌─────────────────────────────────────┐
 * │      GPIO Output (J2/J3 Headers)    │
 * │  ├─ 5x RGB LEDs (puzzle status)    │
 * │  ├─ 2x Status LEDs (power, error)  │
 * │  └─ 1x Buzzer (audio feedback)     │
 * ├─────────────────────────────────────┤
 * │      GPIO Input (PB1, PB2)          │
 * │  ├─ Switch POWER (PB1)              │
 * │  └─ Switch RESET (PB2)              │
 * ├─────────────────────────────────────┤
 * │      GPIO Matrix Input (PA0-PA13)   │
 * │  └─ 4x4 Keypad (J5 Pmod)           │
 * ├─────────────────────────────────────┤
 * │      UART Communication (LPUART0)   │
 * │  └─ Bluetooth Module (HC-05)        │
 * │     TX:PTA3, RX:PTA2, Baud:9600    │
 * ├─────────────────────────────────────┤
 * │      I2C Communication (LPI2C0)     │
 * │  └─ OLED Display SSD1306            │
 * │     SDA:PTC7, SCL:PTC6, 100kHz     │
 * └─────────────────────────────────────┘
 * 
 * Main Loop Flow:
 *   1. Initialize all hardware modules
 *   2. Initialize puzzle box state machine
 *   3. Loop continuously:
 *      a. Read all input devices (debounce)
 *      b. Update puzzle box state machine
 *      c. Update LED displays based on state
 *      d. Update OLED with puzzle status
 *      e. Sleep ~10ms to prevent busy-loop
 * 
 * Puzzle Game Flow:
 *   1. Power on → System IDLE
 *   2. Press POWER button → Puzzle 1 active (Yellow LED)
 *   3. Press keypad '1' / send BT 'A' / etc. → Check if correct
 *      - CORRECT: Green LED, success buzz, advance to Puzzle 2
 *      - WRONG:   Red LED, error buzz, retry Puzzle 1
 *   4. Repeat until Puzzle 5 solved
 *   5. COMPLETE: All LEDs green, victory morse code
 * 
 * Build Instructions for MCUXpresso:
 *   1. Open MCUXpresso IDE
 *   2. Create new project → FRDM-MCXA153
 *   3. Create board support project (includes SDK)
 *   4. Replace main.c with this file
 *   5. Copy header files to inc/ directory
 *   6. Copy source files to src/ directory
 *   7. Add SDK includes:
 *      - #include "fsl_device_registers.h" (GPIO, UART, I2C)
 *      - #include "board.h" (board setup)
 *      - #include "peripherals.h" (periperal config)
 *   8. Build and debug via MCU-Link on board
 */

#include "leds.h"
#include "buzzer.h"
#include "switches.h"
#include "keypad.h"
#include "oled.h"
#include "bt.h"
#include "puzzle_box.h"

// TODO: Add MCUXpresso SDK includes:
// #include "fsl_device_registers.h"
// #include "fsl_gpio.h"
// #include "fsl_lpuart.h"
// #include "fsl_lpi2c.h"

int main(void) {
    // ============================================
    // Initialize Hardware Modules
    // ============================================
    
    // GPIO initialization for LED outputs and button inputs
    leds_init();        // RGB and status LEDs (J2/J3/J4 headers)
    
    // Audio feedback module (Buzzer on PD6, J4 Pin 16)
    buzzer_init();      // Buzzer for morse code, beeps, etc.
    
    // Button inputs (POWER=PB1, RESET=PB2 on J3)
    switches_init();    // Read push buttons with debounce
    
    // Keypad matrix input (4x4 keypad on J5 Pmod header)
    keypad_init();      // PA0-PA13 for 4x4 keypad matrix
    
    // OLED I2C display (SDA=PTC7, SCL=PTC6 on J2 Arduino header)
    oled_init();        // Initialize I2C and OLED SSD1306 display
    
    // Bluetooth wireless input (LPUART0, TX=PTA3, RX=PTA2)
    bt_init();          // Initialize UART for HC-05/HC-06 Bluetooth
    
    // ============================================
    // Initialize Puzzle Box State Machine
    // ============================================
    puzzlebox_init();   // Reset puzzle counters, state = IDLE
    
    
    // ============================================
    // Main Event Loop
    // ============================================
    while (1) {
        // Poll input devices (non-blocking reads)
        uint8_t switch_state = switches_read();  // Check if buttons pressed
        char key = keypad_read();                 // Check if key pressed
        
        // Check Bluetooth for incoming data
        if (bt_available()) {
            char bt_signal = bt_receive();       // Get one character from Bluetooth
            // TODO: Handle bluetooth signal for puzzle logic
        }
        
        // Update puzzle box state machine
        // This updates LEDs, handles timeouts, checks win conditions
        puzzlebox_update();
        
        // Update OLED display with current puzzle status
        // TODO: oled_display_puzzle(puzzlebox_get_current_puzzle());
        
        // Small delay to prevent CPU busy-loop (10ms ~= 100 Hz update rate)
        // TODO: Replace with actual delay function from MCUXpresso SDK
        // TODO: delay_ms(10);
    }
    
    return 0;
}
