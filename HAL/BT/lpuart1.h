#ifndef LPUART1_H
#define LPUART1_H

#include <MCXA153.h>
#include <stdint.h>

void lpuart1_init(const uint32_t baudrate);
void lpuart1_putchar(const int data);
int lpuart1_getchar(void);
uint32_t lpuart1_rxcnt(void);

#endif