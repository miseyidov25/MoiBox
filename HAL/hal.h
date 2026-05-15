#ifndef HAL_H
#define HAL_H

/*
 * ============================================================================
 * FRDM-MCXA153 BOARD PIN DOCUMENTATION
 * ============================================================================
 *
 * This file documents FRDM-MCXA153 pin usage for the iBeacon project and for
 * related FRDM-MCXA153 board examples.
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
 * PORT 3 (P3) - Onboard LEDs, OLED I2C, Buzzer, and Buttons:
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
 *   - The FRDM-MCXA153 board includes one onboard tri-color RGB LED.
 *   - External indicator LEDs are available on breakout pins D1..D6.
 *   - Onboard switches are SW2 and SW3.
 *   - OLED examples use I2C on P3_27/P3_28.
 *   - Buzzer examples use P3_30 as CT0_MAT2.
 *   - LPUART0 on P0_2/P0_3 is used for MCU-Link debug.
 *   - LPUART2 on P1_4/P1_5 is used for external serial/iBeacon data.
 *
 */

#endif // HAL_H