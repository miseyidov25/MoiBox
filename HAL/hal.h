#ifndef HAL_H
#define HAL_H

#include <stdint.h>

void hal_init(void);
void hal_update(uint32_t current_ms);

#endif