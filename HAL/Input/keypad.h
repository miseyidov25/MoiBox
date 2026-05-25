#ifndef KEYPAD_H
#define KEYPAD_H

#include <MCXA153.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Keypad wiring:
 *
 * COL1 -> P2_0
 * COL2 -> P2_1
 * COL3 -> P2_2
 *
 * ROW1 -> P2_4
 * ROW2 -> P2_5
 * ROW3 -> P2_6
 * ROW4 -> P2_7
 */

#define KEYPAD_GPIO GPIO2
#define KEYPAD_PORT PORT2

#define KEYPAD_COL0_PIN 0u
#define KEYPAD_COL1_PIN 1u
#define KEYPAD_COL2_PIN 2u

#define KEYPAD_ROW0_PIN 4u
#define KEYPAD_ROW1_PIN 5u
#define KEYPAD_ROW2_PIN 6u
#define KEYPAD_ROW3_PIN 7u

#define KEYPAD_NUM_COLS 3u
#define KEYPAD_NUM_ROWS 4u

void keypad_init(void);
void keypad_update(uint32_t current_ms);
bool keypad_getkey(char *key);

#endif