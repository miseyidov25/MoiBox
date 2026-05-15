#include "hal.h"

/*
 * ============================================================================
 * FRDM-MCXA153 BOARD PIN DOCUMENTATION
 * ============================================================================
 *
 * This file is documentation-only. It captures the FRDM-MCXA153 pin mapping
 * that is relevant for the iBeacon project and for common FRDM-MCXA153 board
 * examples.
 *
 * The FRDM-MCXA153 board provides an onboard RGB LED, two user switches,
 * a buzzer output pin, and external I2C pins for displays/peripherals.
 * Additional normal LEDs and buttons can be wired to expansion header pins.
 *
 * PORT 0 (P0) - Debug UART:
 * ------------------------
 * P0_2 : LPUART0_RX - MCU-Link debug serial RX
 * P0_3 : LPUART0_TX - MCU-Link debug serial TX
 *
 * PORT 1 (P1) - Application UART and User Button:
 * ----------------------------------------------
 * P1_4 : LPUART2_RXD - Application serial RX for external device
 * P1_5 : LPUART2_TXD - Application serial TX for external device
 * P1_7 : BUTTON_SW3  - Onboard switch SW3 (GPIO input)
 *
 * PORT 2 (P2) - Breakout LEDs:
 * ---------------------------
 * P2_12 : D1 - Breakout indicator LED 1
 * P2_16 : D2 - Breakout indicator LED 2
 * P2_13 : D3 - Breakout indicator LED 3
 * P2_6  : D4 - Breakout indicator LED 4
 *
 * PORT 3 (P3) - Onboard LEDs, OLED I2C, Buzzer, and Button:
 * --------------------------------------------------------
 * P3_0  : RGB_LED_BLUE  - Onboard RGB LED blue channel
 * P3_12 : RGB_LED_GREEN - Onboard RGB LED green channel
 * P3_13 : RGB_LED_RED   - Onboard RGB LED red channel
 * P3_14 : D5           - Breakout indicator LED 5
 * P3_15 : D6           - Breakout indicator LED 6
 * P3_27 : OLED_SCL      - OLED / I2C clock (LPI2C0_SCL)
 * P3_28 : OLED_SDA      - OLED / I2C data  (LPI2C0_SDA)
 * P3_29 : BUTTON_SW2    - Onboard switch SW2 (GPIO input)
 * P3_30 : BUZZER_OUT    - Buzzer output pin (CT0_MAT2)
 *
 *
 * Notes:
 *   - The onboard FRDM-MCXA153 RGB LED is a single tri-color LED.
 *   - Additional indicator LEDs are available on breakout pins D1..D6.
 *   - Onboard switches are SW2 and SW3 only.
 *   - OLED examples for FRDM-MCXA153 typically use I2C on P3_27/P3_28.
 *   - Buzzer examples use P3_30 as CT0_MAT2 PWM output.
 *   - LPUART0 on P0_2/P0_3 is connected to MCU-Link for debug.
 *   - LPUART2 on P1_4/P1_5 is used for external serial/iBeacon data.
 *
 *
 * ============================================================================
 * DEVICE CONNECTIONS - iBeacon Application
 * ============================================================================
 *
 * USB/Debug Interface (via MCU-Link):
 * ----------------------------------
 * LPUART0 on P0_2 (RX) and P0_3 (TX)
 *   - Connected to MCU-Link for debug and host console output.
 *   - Baud rate: 115200.
 *
 * External iBeacon/Serial Device:
 * ------------------------------
 * LPUART2 on P1_4 (RXD) and P1_5 (TXD)
 *   - Receives iBeacon frames from an external radio module.
 *   - Baud rate: 115200.
 *
 * User Interface:
 * ---------------
 * OLED Display:
 *   - Uses I2C on P3_27 (SCL) and P3_28 (SDA) on FRDM-MCXA153.
 *
 * RGB LED:
 *   - The board's single tri-color RGB LED uses P3_12, P3_13, and P3_0.
 *   - Used to display status, distance, and mode information.
 *
 * Normal LEDs:
 *   - Breakout LEDs D1..D6 are available on P2_12, P2_16, P2_13, P2_6,
 *     P3_14, and P3_15.
 *
 * Buttons:
 *   - Onboard switches SW2 and SW3 use P3_29 and P1_7 respectively.
 *
 * Buzzer:
 *   - Buzzer output is available on P3_30 using CT0_MAT2.
 *
 *
 * Current iBeacon implementation notes:
 *   - The project currently implements LPUART0 and LPUART2 serial ports.
 *   - The onboard RGB LED is used for status indication.
 *   - The current button driver is implemented for SW2 on P3_29.
 *   - OLED and buzzer support are documented as FRDM-MCXA153 resources.
 *
 */
