#ifndef SWITCHES_H
#define SWITCHES_H

#include <stdint.h>

// bit positions for different buttons/switches
#define SWITCH_BIT_POWER   (1u<<0)
#define SWITCH_BIT_RESET   (1u<<1)
#define SWITCH_BIT_NEXT    (1u<<2)
#define SWITCH_BIT_PREV    (1u<<3)
#define SWITCH_BIT_SELECT  (1u<<4)

void switches_init(void);           // configure pins as inputs
uint8_t switches_read(void);        // return mask of active switches

#endif // SWITCHES_H
