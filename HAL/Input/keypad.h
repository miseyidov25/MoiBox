#ifndef KEYPAD_H
#define KEYPAD_H

#include <MCXA153.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * ROWS outputs:
 * R1 -> P2_0
 * R2 -> P2_1
 * R3 -> P2_2
 * R4 -> P2_3
 *
 * COLUMNS inputs with pull-ups:
 * C1 -> P1_8
 * C2 -> P1_9
 * C3 -> P1_12
 */

void keypad_init(void);
void keypad_update(uint32_t current_ms);
bool keypad_getkey(char *key);

#endif