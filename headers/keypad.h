#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>


void keypad_init(void);
char keypad_read(void);  // return key or 0 if none

#endif // KEYPAD_H
