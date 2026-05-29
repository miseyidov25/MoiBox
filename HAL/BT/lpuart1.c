#include "HAL/BT/lpuart1.h"

#include "fifo.h"

static fifo_t tx;
static fifo_t rx;

static uint8_t tx_buffer[1024];
static uint8_t rx_buffer[1024];

void lpuart1_init(const uint32_t baudrate)
{
    f_init(&tx, tx_buffer, sizeof(tx_buffer), sizeof(uint8_t));
    f_init(&rx, rx_buffer, sizeof(rx_buffer), sizeof(uint8_t));

    /*
     * Clock source: FRO_HF_DIV, normally 48 MHz.
     */
    MRCC0->MRCC_LPUART1_CLKSEL = MRCC_MRCC_LPUART1_CLKSEL_MUX(0b010);
    MRCC0->MRCC_LPUART1_CLKDIV = 0u;

    MRCC0->MRCC_GLB_CC0_SET =
        MRCC_MRCC_GLB_CC0_LPUART1(1);

    MRCC0->MRCC_GLB_CC1_SET =
        MRCC_MRCC_GLB_CC1_PORT3(1);

    MRCC0->MRCC_GLB_RST0_SET =
        MRCC_MRCC_GLB_RST0_LPUART1(1);

    MRCC0->MRCC_GLB_RST1_SET =
        MRCC_MRCC_GLB_RST1_PORT3(1);

    /*
     * HM-10:
     *
     * TXD -> P3_8 / LPUART1_RXD / ALT3
     * RXD -> P3_9 / LPUART1_TXD / ALT3
     */
    PORT3->PCR[8] =
        PORT_PCR_MUX(3) |
        PORT_PCR_IBE(1);

    PORT3->PCR[9] =
        PORT_PCR_MUX(3);

    LPUART1->CTRL = 0u;

    LPUART1->BAUD =
        LPUART_BAUD_OSR(0b01111) |
        LPUART_BAUD_SBR(48000000u / (baudrate * 16u));

    LPUART1->CTRL =
        LPUART_CTRL_TE(1) |
        LPUART_CTRL_RIE(1) |
        LPUART_CTRL_RE(1);

    NVIC_SetPriority(LPUART1_IRQn, 3);
    NVIC_ClearPendingIRQ(LPUART1_IRQn);
    NVIC_EnableIRQ(LPUART1_IRQn);
}

void lpuart1_putchar(const int data)
{
    uint8_t c = (uint8_t)data;

    while (!f_push(&tx, &c))
    {
    }

    LPUART1->CTRL |= LPUART_CTRL_TIE(1);
}

int lpuart1_getchar(void)
{
    uint8_t c = 0u;

    while (!f_pop(&rx, &c))
    {
    }

    return (int)c;
}

uint32_t lpuart1_rxcnt(void)
{
    return f_cnt(&rx);
}

void LPUART1_IRQHandler(void)
{
    uint8_t c;

    NVIC_ClearPendingIRQ(LPUART1_IRQn);

    if ((LPUART1->STAT & LPUART_STAT_TDRE_MASK) != 0u)
    {
        if (f_pop(&tx, &c))
        {
            LPUART1->DATA = c;
        }
        else
        {
            LPUART1->CTRL &= ~(LPUART_CTRL_TIE(1));
        }
    }

    if ((LPUART1->STAT & LPUART_STAT_RDRF_MASK) != 0u)
    {
        c = (uint8_t)LPUART1->DATA;

        /*
         * If RX FIFO is full, discard byte instead of freezing project.
         */
        (void)f_push(&rx, &c);
    }
}