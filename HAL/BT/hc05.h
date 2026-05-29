#ifndef HC05_H
#define HC05_H

#include <stdint.h>
#include <stdbool.h>

void hc05_init(uint32_t baudrate);
void hc05_update(void);

bool hc05_getchar(char *c);
uint32_t hc05_rxcnt(void);

void hc05_putchar(char c);
void hc05_write_string(const char *s);

#endif