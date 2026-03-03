#ifndef PINS_H
#define PINS_H

/*
 * FRDM-MCXA153 Development Board Pin Configuration
 * ====================================================
 * 
 * Board Connector Reference:
 * - J2: Arduino-style header (top)
 * - J3: Arduino-style header (bottom)  
 * - J4/J5: FRDM/Pmod headers (sides)
 * - MCU: MCXA153 (Arm Cortex-M33) - 64-pin LQFP
 * 
 * Port mapping (MCX A153):
 * PTA = Port A (GPIO pins)
 * PTB = Port B (GPIO pins)
 * PTC = Port C (GPIO pins)
 * PTD = Port D (GPIO pins)
 * PTE = Port E (GPIO pins)
 */

// ============================================
// RGB LED OUTPUTS - 5 Sets (Puzzle 1-5 Status)
// ============================================
// Location: J2 Arduino headers (rows 1-10 on left/right,
//           or external expansion via breadboard/shield)
// 
// RGB LED 1 (Puzzle 1) - PTD pins
#define RGB_LED1_R  kGPIO_PORTD, 0u    // PD0, J2 Pin 9
#define RGB_LED1_G  kGPIO_PORTD, 1u    // PD1, J2 Pin 8
#define RGB_LED1_B  kGPIO_PORTD, 2u    // PD2, J2 Pin 7

// RGB LED 2 (Puzzle 2) - PTD pins continued
#define RGB_LED2_R  kGPIO_PORTD, 3u    // PD3, J2 Pin 6
#define RGB_LED2_G  kGPIO_PORTD, 4u    // PD4, J2 Pin 5
#define RGB_LED2_B  kGPIO_PORTD, 5u    // PD5, J2 Pin 4

// RGB LED 3 (Puzzle 3) - PTC pins
#define RGB_LED3_R  kGPIO_PORTC, 0u    // PC0, J2 Pin 3
#define RGB_LED3_G  kGPIO_PORTC, 1u    // PC1, J2 Pin 2
#define RGB_LED3_B  kGPIO_PORTC, 2u    // PC2, J2 Pin 1

// RGB LED 4 (Puzzle 4) - PTC pins continued
#define RGB_LED4_R  kGPIO_PORTC, 3u    // PC3, J3 Pin 1
#define RGB_LED4_G  kGPIO_PORTC, 4u    // PC4, J3 Pin 2
#define RGB_LED4_B  kGPIO_PORTC, 5u    // PC5, J3 Pin 3

// RGB LED 5 (Puzzle 5) - PTE pins
#define RGB_LED5_R  kGPIO_PORTE, 0u    // PE0, J3 Pin 4
#define RGB_LED5_G  kGPIO_PORTE, 1u    // PE1, J3 Pin 5
#define RGB_LED5_B  kGPIO_PORTE, 2u    // PE2, J3 Pin 6

// ============================================
// STATUS LEDs - Power/Error Indicators
// ============================================
// Location: J4 FRDM header or external via jumper wires
//
#define LED_POWER   kGPIO_PORTE, 3u    // PE3, J4 Pin 14
#define LED_ERROR   kGPIO_PORTB, 0u    // PB0, J4 Pin 12

// ============================================
// SWITCHES/BUTTONS - Reset, Power Control
// ============================================
// Location: J3 right side pins / external buttons
//
#define SWITCH_POWER    kGPIO_PORTB, 1u    // PB1, J3 Pin 7 (with pullup)
#define SWITCH_RESET    kGPIO_PORTB, 2u    // PB2, J3 Pin 8 (with pullup)

// ============================================
// BUZZER - Audio Feedback (PWM capable)
// ============================================
// Location: J4 FRDM header, PTD6 (TPM0 PWM module)
//
#define BUZZER_PIN     kGPIO_PORTD, 6u    // PD6, J4 Pin 16 (PWM capable)

// ============================================
// KEYPAD - 4x4 Matrix (16 buttons)
// ============================================
// Layout:
//   1 2 3 A
//   4 5 6 B
//   7 8 9 C
//   * 0 # D
//
// Rows (Select which row to read) - PTA pins
// Location: J5 Pmod header
//
#define KEYPAD_ROW1  kGPIO_PORTA, 0u   // PA0, J5 Pin 1
#define KEYPAD_ROW2  kGPIO_PORTA, 1u   // PA1, J5 Pin 2
#define KEYPAD_ROW3  kGPIO_PORTA, 2u   // PA2, J5 Pin 3
#define KEYPAD_ROW4  kGPIO_PORTA, 3u   // PA3, J5 Pin 4

// Columns (Read which columns are pressed) - PTA pins continued
//
#define KEYPAD_COL1  kGPIO_PORTA, 4u   // PA4, J5 Pin 5
#define KEYPAD_COL2  kGPIO_PORTA, 5u   // PA5, J5 Pin 6
#define KEYPAD_COL3  kGPIO_PORTA, 12u  // PA12, J5 Pin 7
#define KEYPAD_COL4  kGPIO_PORTA, 13u  // PA13, J5 Pin 8

// ============================================
// OLED DISPLAY - I2C SSD1306 (128x64 pixels)
// ============================================
// Protocol: LPI2C0 (I2C module 0)
// Standard pins on Arduino header
// Location: J2/J3 Arduino I2C pins
//
#define OLED_I2C_MODULE        LPI2C0      // I2C Module 0
#define OLED_SDA_PORT          kGPIO_PORTC // SDA on PTC7
#define OLED_SDA_PIN           7u
#define OLED_SCL_PORT          kGPIO_PORTC // SCL on PTC6
#define OLED_SCL_PIN           6u
#define OLED_I2C_FREQ          100000u     // 100 kHz I2C speed
// Note: I2C pins are on J2 Arduino header (pins D18=SDA, D19=SCL by convention)

// ============================================
// BLUETOOTH MODULE - HC-05/HC-06 via UART
// ============================================
// Protocol: LPUART0 (Low-Power UART)
// Baud rate: 9600 (standard for HC-05/HC-06)
// Location: J4 FRDM header
//
#define BT_UART_MODULE        LPUART0    // UART Module 0
#define BT_RX_PORT            kGPIO_PORTA // RX on PTA2
#define BT_RX_PIN             2u
#define BT_TX_PORT            kGPIO_PORTA // TX on PTA3
#define BT_TX_PIN             3u
#define BT_BAUD_RATE          9600       // HC-05/HC-06 default baud rate

// Note: Bluetooth module power (5V/GND) from J4 power pins

// ============================================
// BOARD POWER & GND DISTRIBUTION
// ============================================
// VCC (3.3V) - Multiple available on J2, J3, J4, J5
//   - Can power OLED, Bluetooth module (with voltage divider), etc.
// GND (Ground) - Multiple available on all headers
//   - Common ground for all peripherals

// ============================================
// NOTES FOR FRDM-MCXA153
// ============================================
// - Board has 64-pin LQFP MCX A153 microcontroller
// - All GPIO pins are 3.3V (NOT 5V tolerant!)
// - Avoid PTA0/PTA1 - used by debugger
// - Avoid PC0/PC1/PC2 - may be reserved in some configurations
// - For 5V devices (Bluetooth module), use level shifter or voltage divider
// - I2C pull-ups may be needed depending on OLED module (usually built-in 10k)

#endif // PINS_H
