#include "keypad.h"
#include "pins.h"

static const char keypad_map[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

void keypad_init(void) {
    // configure row pins as outputs, col pins as inputs
}

char keypad_read(void) {
    // simple placeholder scan: always return 0
    // real code would drive each row low and read columns
    return 0;
}

