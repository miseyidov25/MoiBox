#ifndef APP_H
#define APP_H

#include <stdint.h>

extern volatile uint32_t ms;

void app_init(void);
void app_update(void);

void app_tick_1ms(void);
uint32_t app_millis(void);

#endif