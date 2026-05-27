#ifndef BOARD_PINS_H
#define BOARD_PINS_H

/*
 * ============================================================
 * MoiBox pin map - FRDM-MCXA153
 * ============================================================
 *
 * This file is a central reference for all project pins.
 *
 * Current pin plan has no direct GPIO conflicts:
 *
 * P0: USB debug serial only
 * P1: SD + BT + colored buttons + some map LEDs
 * P2: keypad + lock + some map LEDs
 * P3: OLED + map LEDs + optional buzzer
 */

/*
 * ============================================================
 * USB DEBUG SERIAL
 * ============================================================
 *
 * Used by serial.c / LPUART0 / MCU-Link.
 *
 * MCU-Link TX -> P0_2 / LPUART0_RX
 * MCU-Link RX -> P0_3 / LPUART0_TX
 */

#define PIN_DEBUG_UART_RX_PORT_NUMBER 0u
#define PIN_DEBUG_UART_RX_PIN         2u

#define PIN_DEBUG_UART_TX_PORT_NUMBER 0u
#define PIN_DEBUG_UART_TX_PIN         3u

/*
 * ============================================================
 * SD CARD / HW-125 MODULE
 * ============================================================
 *
 * Uses LPSPI0.
 *
 * HW-125 MOSI -> P1_0 / LPSPI0_SDO
 * HW-125 SCK  -> P1_1 / LPSPI0_SCK
 * HW-125 MISO -> P1_2 / LPSPI0_SDI
 * HW-125 CS   -> P1_3 / manual GPIO CS
 *
 * HW-125 VCC  -> 3.3V recommended for your current setup
 * HW-125 GND  -> GND
 */

#define PIN_SD_MOSI_PORT_NUMBER 1u
#define PIN_SD_MOSI_PIN         0u

#define PIN_SD_SCK_PORT_NUMBER  1u
#define PIN_SD_SCK_PIN          1u

#define PIN_SD_MISO_PORT_NUMBER 1u
#define PIN_SD_MISO_PIN         2u

#define PIN_SD_CS_PORT_NUMBER   1u
#define PIN_SD_CS_PIN           3u

/*
 * ============================================================
 * BLUETOOTH / HM-10
 * ============================================================
 *
 * Uses LPUART2.
 *
 * HM-10 TXD -> P1_4 / LPUART2_RX
 * HM-10 RXD -> P1_5 / LPUART2_TX
 * HM-10 VCC -> 3.3V
 * HM-10 GND -> GND
 *
 * Current code uses BT_PIN_MUX = 2.
 * If HM-10 still does not respond, test MUX 3 and baud 115200.
 */

#define PIN_BT_UART_RX_PORT_NUMBER 1u
#define PIN_BT_UART_RX_PIN         4u

#define PIN_BT_UART_TX_PORT_NUMBER 1u
#define PIN_BT_UART_TX_PIN         5u

/*
 * ============================================================
 * COLORED BUTTONS
 * ============================================================
 *
 * Active-low buttons:
 *
 * not pressed = HIGH
 * pressed     = LOW
 *
 * Other side of each button goes to GND.
 */

#define PIN_BUTTON_YELLOW_PORT_NUMBER 1u
#define PIN_BUTTON_YELLOW_PIN         6u

#define PIN_BUTTON_BLUE_PORT_NUMBER   1u
#define PIN_BUTTON_BLUE_PIN           7u

#define PIN_BUTTON_GREEN_PORT_NUMBER  1u
#define PIN_BUTTON_GREEN_PIN          8u

#define PIN_BUTTON_RED_PORT_NUMBER    1u
#define PIN_BUTTON_RED_PIN            9u

/*
 * ============================================================
 * KEYPAD 3x4
 * ============================================================
 *
 * Fully on PORT2 to avoid conflicts.
 *
 * COL1 -> P2_0
 * COL2 -> P2_1
 * COL3 -> P2_2
 *
 * ROW1 -> P2_4
 * ROW2 -> P2_5
 * ROW3 -> P2_6
 * ROW4 -> P2_7
 *
 * P2_3 intentionally left free.
 */

#define PIN_KEYPAD_COL1_PORT_NUMBER 2u
#define PIN_KEYPAD_COL1_PIN         0u

#define PIN_KEYPAD_COL2_PORT_NUMBER 2u
#define PIN_KEYPAD_COL2_PIN         1u

#define PIN_KEYPAD_COL3_PORT_NUMBER 2u
#define PIN_KEYPAD_COL3_PIN         2u

#define PIN_KEYPAD_ROW1_PORT_NUMBER 2u
#define PIN_KEYPAD_ROW1_PIN         4u

#define PIN_KEYPAD_ROW2_PORT_NUMBER 2u
#define PIN_KEYPAD_ROW2_PIN         5u

#define PIN_KEYPAD_ROW3_PORT_NUMBER 2u
#define PIN_KEYPAD_ROW3_PIN         6u

#define PIN_KEYPAD_ROW4_PORT_NUMBER 2u
#define PIN_KEYPAD_ROW4_PIN         7u

/*
 * ============================================================
 * LOCK CONTROL
 * ============================================================
 *
 * P2_10 controls a MOSFET/transistor/relay module.
 *
 * The GPIO does NOT power the lock directly.
 * Lock opens when power is provided.
 */

#define PIN_LOCK_PORT_NUMBER 2u
#define PIN_LOCK_PIN         10u

/*
 * ============================================================
 * OLED DISPLAY
 * ============================================================
 *
 * Uses LPI2C0.
 *
 * OLED SCL -> P3_27 / LPI2C0_SCL
 * OLED SDA -> P3_28 / LPI2C0_SDA
 * OLED VCC -> 3.3V
 * OLED GND -> GND
 */

#define PIN_OLED_SCL_PORT_NUMBER 3u
#define PIN_OLED_SCL_PIN         27u

#define PIN_OLED_SDA_PORT_NUMBER 3u
#define PIN_OLED_SDA_PIN         28u

/*
 * ============================================================
 * LOCATION RGB MAP LEDs
 * ============================================================
 *
 * Your RGB LEDs currently use only RED and GREEN channels.
 *
 * red    = red channel ON
 * green  = green channel ON
 * yellow = red + green ON
 *
 * Common GND, active-high outputs.
 */

/* RGB location 1 */
#define PIN_RGB1_RED_PORT_NUMBER   3u
#define PIN_RGB1_RED_PIN           10u

#define PIN_RGB1_GREEN_PORT_NUMBER 3u
#define PIN_RGB1_GREEN_PIN         11u

/* RGB location 2 */
#define PIN_RGB2_RED_PORT_NUMBER   3u
#define PIN_RGB2_RED_PIN           13u

#define PIN_RGB2_GREEN_PORT_NUMBER 3u
#define PIN_RGB2_GREEN_PIN         14u

/* RGB location 3 */
#define PIN_RGB3_RED_PORT_NUMBER   2u
#define PIN_RGB3_RED_PIN           12u

#define PIN_RGB3_GREEN_PORT_NUMBER 2u
#define PIN_RGB3_GREEN_PIN         13u

/* RGB location 4 */
#define PIN_RGB4_RED_PORT_NUMBER   1u
#define PIN_RGB4_RED_PIN           10u

#define PIN_RGB4_GREEN_PORT_NUMBER 1u
#define PIN_RGB4_GREEN_PIN         11u

/* RGB location 5 */
#define PIN_RGB5_RED_PORT_NUMBER   1u
#define PIN_RGB5_RED_PIN           13u

#define PIN_RGB5_GREEN_PORT_NUMBER 3u
#define PIN_RGB5_GREEN_PIN         0u

/*
 * ============================================================
 * OPTIONAL / FUTURE: 4 NORMAL COLORED LEDS
 * ============================================================
 *
 * These were listed earlier for puzzle 4/5 feedback.
 * They are NOT clearly implemented in the current pasted code.
 *
 * Previous candidate mapping:
 *
 * RED normal LED    -> P3_9
 * GREEN normal LED  -> P3_6
 * BLUE normal LED   -> P3_7
 * YELLOW normal LED -> P3_8
 *
 * WARNING:
 * P3_6 was also suggested by an older buzzer branch.
 * Do not use P3_6 for both normal green LED and buzzer.
 */

#define PIN_NORMAL_LED_RED_PORT_NUMBER    3u
#define PIN_NORMAL_LED_RED_PIN            9u

#define PIN_NORMAL_LED_GREEN_PORT_NUMBER  3u
#define PIN_NORMAL_LED_GREEN_PIN          6u

#define PIN_NORMAL_LED_BLUE_PORT_NUMBER   3u
#define PIN_NORMAL_LED_BLUE_PIN           7u

#define PIN_NORMAL_LED_YELLOW_PORT_NUMBER 3u
#define PIN_NORMAL_LED_YELLOW_PIN         8u

/*
 * ============================================================
 * BUZZER
 * ============================================================
 *
 * Current buzzer.c uses P3_5 but BUZZER_ENABLED is 0.
 *
 * Use P3_5 if available.
 * Do NOT use P3_6 if you also use the normal green LED above.
 */

#define PIN_BUZZER_PORT_NUMBER 3u
#define PIN_BUZZER_PIN         5u

/*
 * ============================================================
 * FREE / RESERVED PINS IN THIS PLAN
 * ============================================================
 *
 * Known intentionally free:
 *
 * P2_3
 * P2_8
 * P2_9
 * P2_11
 *
 * Check board headers before using other pins.
 */

/*
 * ============================================================
 * SUMMARY BY PORT
 * ============================================================
 *
 * PORT0:
 * P0_2  = debug UART RX
 * P0_3  = debug UART TX
 *
 * PORT1:
 * P1_0  = SD MOSI
 * P1_1  = SD SCK
 * P1_2  = SD MISO
 * P1_3  = SD CS
 * P1_4  = BT RX
 * P1_5  = BT TX
 * P1_6  = yellow button
 * P1_7  = blue button
 * P1_8  = green button
 * P1_9  = red button
 * P1_10 = RGB4 red
 * P1_11 = RGB4 green
 * P1_13 = RGB5 red
 *
 * PORT2:
 * P2_0  = keypad COL1
 * P2_1  = keypad COL2
 * P2_2  = keypad COL3
 * P2_4  = keypad ROW1
 * P2_5  = keypad ROW2
 * P2_6  = keypad ROW3
 * P2_7  = keypad ROW4
 * P2_10 = lock control
 * P2_12 = RGB3 red
 * P2_13 = RGB3 green
 *
 * PORT3:
 * P3_0  = RGB5 green
 * P3_5  = buzzer
 * P3_6  = optional normal green LED
 * P3_7  = optional normal blue LED
 * P3_8  = optional normal yellow LED
 * P3_9  = optional normal red LED
 * P3_10 = RGB1 red
 * P3_11 = RGB1 green
 * P3_13 = RGB2 red
 * P3_14 = RGB2 green
 * P3_27 = OLED SCL
 * P3_28 = OLED SDA
 */

#endif