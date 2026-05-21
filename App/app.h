#ifndef APP_H
#define APP_H

#include <stdint.h>

void app_init(void);
void app_update(void);

void app_tick_1ms(void);
uint32_t app_millis(void);

#endif