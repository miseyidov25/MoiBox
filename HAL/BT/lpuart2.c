#include "HAL/BT/lpuart2.h"

#include "fifo.h"

static fifo_t tx;
static fifo_t rx;

static uint8_t tx_buffer[1024];
static uint8_t rx_buffer[1024];

void lpuart2_init(const uint32_t baudrate)
{
    f_init(&tx, tx_buffer, sizeof(tx_buffer), sizeof(uint8_t));
    f_init(&rx, rx_buffer, sizeof(rx_buffer), sizeof(uint8_t));

    /*
     * Same setup as working serial2lpuart project.
     * Clock source: FRO_HF_DIV, normally 48 MHz.
     */
    MRCC0->MRCC_LPUART2_CLKSEL = MRCC_MRCC_LPUART2_CLKSEL_MUX(0b010);
    MRCC0->MRCC_LPUART2_CLKDIV = 0;

    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_LPUART2(1);
    MRCC0->MRCC_GLB_CC0_SET = MRCC_MRCC_GLB_CC0_PORT1(1);

    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_LPUART2(1);
    MRCC0->MRCC_GLB_RST0_SET = MRCC_MRCC_GLB_RST0_PORT1(1);

    /*
     * HM-10:
     * TXD -> P1_4 / LPUART2_RXD / ALT3
     * RXD -> P1_5 / LPUART2_TXD / ALT3
     */
    PORT1->PCR[4] =
        PORT_PCR_MUX(3) |
        PORT_PCR_IBE(1);

    PORT1->PCR[5] =
        PORT_PCR_MUX(3);

    LPUART2->CTRL = 0u;

    LPUART2->BAUD =
        LPUART_BAUD_OSR(0b01111) |
        LPUART_BAUD_SBR(48000000u / (baudrate * 16u));

    LPUART2->CTRL =
        LPUART_CTRL_TE(1) |
        LPUART_CTRL_RIE(1) |
        LPUART_CTRL_RE(1);

    NVIC_SetPriority(LPUART2_IRQn, 3);
    NVIC_ClearPendingIRQ(LPUART2_IRQn);
    NVIC_EnableIRQ(LPUART2_IRQn);
}

void lpuart2_putchar(const int data)
{
    uint8_t c = (uint8_t)data;

    while (!f_push(&tx, &c))
    {
    }

    LPUART2->CTRL |= LPUART_CTRL_TIE(1);
}

int lpuart2_getchar(void)
{
    uint8_t c = 0u;

    while (!f_pop(&rx, &c))
    {
    }

    return (int)c;
}

uint32_t lpuart2_rxcnt(void)
{
    return f_cnt(&rx);
}

void LPUART2_IRQHandler(void)
{
    uint8_t c;

    NVIC_ClearPendingIRQ(LPUART2_IRQn);

    if ((LPUART2->STAT & LPUART_STAT_TDRE_MASK) != 0u)
    {
        if (f_pop(&tx, &c))
        {
            LPUART2->DATA = c;
        }
        else
        {
            LPUART2->CTRL &= ~(LPUART_CTRL_TIE(1));
        }
    }

    if ((LPUART2->STAT & LPUART_STAT_RDRF_MASK) != 0u)
    {
        c = (uint8_t)LPUART2->DATA;

        if (!f_push(&rx, &c))
        {
            /*
             * RX FIFO full.
             * Do not freeze whole project; discard byte instead.
             */
        }
    }
}