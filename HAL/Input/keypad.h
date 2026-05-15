#ifndef KEYPAD_H
#define KEYPAD_H

#include <MCXA153.h>
#include <stdint.h>
#include <stdbool.h>

// Keypad Hardware Configuration
#define KEYPAD_COL_GPIO          GPIO2
#define KEYPAD_ROW_GPIO          GPIO2
#define KEYPAD_COL_PORT          PORT2
#define KEYPAD_ROW_PORT          PORT2

// Column pins (outputs) - P2.0, P2.1, P2.2
#define KEYPAD_COL0_PIN          0U
#define KEYPAD_COL1_PIN          1U
#define KEYPAD_COL2_PIN          2U

// Row pins (inputs) - P2.4, P2.5, P2.6, P2.7
#define KEYPAD_ROW0_PIN          4U
#define KEYPAD_ROW1_PIN          5U
#define KEYPAD_ROW2_PIN          6U
#define KEYPAD_ROW3_PIN          7U

#define KEYPAD_NUM_COLS          3U
#define KEYPAD_NUM_ROWS          4U

// Keypad character map
// Layout:
//  1  2  3
//  4  5  6
//  7  8  9
//  *  0  #
#define KEYPAD_MAP \
    { \
        {'1', '2', '3'}, \
        {'4', '5', '6'}, \
        {'7', '8', '9'}, \
        {'*', '0', '#'} \
    }

// Function declarations
void keypad_init(void);
bool keypad_getkey(char *key);

#endif // KEYPAD_H
