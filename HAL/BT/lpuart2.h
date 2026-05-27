#ifndef LPUART2_H
#define LPUART2_H

#include <MCXA153.h>
#include <stdint.h>

void lpuart2_init(const uint32_t baudrate);
void lpuart2_putchar(const int data);
int lpuart2_getchar(void);
uint32_t lpuart2_rxcnt(void);

#endif