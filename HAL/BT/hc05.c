#include "HAL/BT/hc05.h"

#include "HAL/BT/lpuart2.h"

void hc05_init(uint32_t baudrate)
{
    /*
     * HC-05 uses LPUART2.
     *
     * Wiring:
     * HC-05 TXD -> P1_4 / LPUART2_RXD
     * HC-05 RXD -> P1_5 / LPUART2_TXD
     */
    lpuart2_init(baudrate);
}

void hc05_update(void)
{

}

bool hc05_getchar(char *c)
{
    if (c == 0)
    {
        return false;
    }

    if (lpuart2_rxcnt() == 0u)
    {
        return false;
    }

    *c = (char)lpuart2_getchar();
    return true;
}

uint32_t hc05_rxcnt(void)
{
    return lpuart2_rxcnt();
}

void hc05_putchar(char c)
{
    lpuart2_putchar((int)c);
}

void hc05_write_string(const char *s)
{
    if (s == 0)
    {
        return;
    }

    while (*s != '\0')
    {
        hc05_putchar(*s);
        s++;
    }
}