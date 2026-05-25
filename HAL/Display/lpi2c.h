#ifndef LPI2C_H
#define LPI2C_H

#include <stdbool.h>
#include <stdint.h>

extern volatile uint32_t lpi2c_error_step;

void lpi2c_controller_init(void);
bool lpi2c_init(void);
bool lpi2c_busy(void);


bool lpi2c_write(uint8_t dev_address, uint8_t reg, const uint8_t *p, uint32_t len);

#endif