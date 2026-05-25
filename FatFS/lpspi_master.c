#include "FatFS/lpspi_master.h"

static uint8_t dummy_byte = 0xFF;

static bool lpspi0_txfifo_full(void)
{
    uint32_t n;

    n = (LPSPI0->FSR & LPSPI_FSR_TXCOUNT_MASK) >> LPSPI_FSR_TXCOUNT_SHIFT;

    return n == 4u;
}

static bool lpspi0_rxfifo_empty(void)
{
    uint32_t n;

    n = (LPSPI0->FSR & LPSPI_FSR_RXCOUNT_MASK) >> LPSPI_FSR_RXCOUNT_SHIFT;

    return n == 0u;
}

void lpspi_master_init(void)
{
    /*
     * LPSPI0 pins:
     * P1_0 = LPSPI0_SDO = MOSI
     * P1_1 = LPSPI0_SCK = SCK
     * P1_2 = LPSPI0_SDI = MISO
     *
     * P1_3 is NOT configured here.
     * P1_3 is manual GPIO CS in diskio.c.
     */

    MRCC0->MRCC_LPSPI0_CLKSEL = MRCC_MRCC_LPSPI0_CLKSEL_MUX(0);
    MRCC0->MRCC_LPSPI0_CLKDIV = 0u;

    MRCC0->MRCC_GLB_CC0_SET =
        MRCC_MRCC_GLB_CC0_LPSPI0(1) |
        MRCC_MRCC_GLB_CC0_PORT1(1);

    MRCC0->MRCC_GLB_RST0_SET =
        MRCC_MRCC_GLB_RST0_LPSPI0(1) |
        MRCC_MRCC_GLB_RST0_PORT1(1);

    PORT1->PCR[0] = PORT_PCR_MUX(2) | PORT_PCR_IBE(1);
    PORT1->PCR[1] = PORT_PCR_MUX(2) | PORT_PCR_IBE(1);
    PORT1->PCR[2] = PORT_PCR_MUX(2) | PORT_PCR_IBE(1);

    LPSPI0->CR = LPSPI_CR_RST(1);
    LPSPI0->CR = 0u;

    LPSPI0->CFGR1 = LPSPI_CFGR1_MASTER(1);

    /*
     * Slow SPI for SD initialization.
     */
    LPSPI0->CCR =
        LPSPI_CCR_SCKPCS(5) |
        LPSPI_CCR_PCSSCK(5) |
        LPSPI_CCR_DBT(10) |
        LPSPI_CCR_SCKDIV(14);

    LPSPI0->CCR1 =
        LPSPI_CCR1_SCKSCK(5) |
        LPSPI_CCR1_PCSPCS(5) |
        LPSPI_CCR1_SCKHLD(5) |
        LPSPI_CCR1_SCKSET(5);

    LPSPI0->CR = LPSPI_CR_MEN(1);
}

bool lpspi_busy(void)
{
    return (LPSPI0->SR & LPSPI_SR_MBF_MASK) != 0u;
}

void lpspi_transmit(uint8_t *tx_buffer, const uint32_t n)
{
    uint32_t timeout;

    if (tx_buffer == 0 || n == 0u)
    {
        return;
    }

    timeout = 1000000u;

    while (lpspi_busy() && timeout > 0u)
    {
        timeout--;
    }

    if (timeout == 0u)
    {
        return;
    }

    LPSPI0->CR |= LPSPI_CR_RTF(1) | LPSPI_CR_RRF(1);

    LPSPI0->SR =
        LPSPI_SR_MBF_MASK |
        LPSPI_SR_DMF_MASK |
        LPSPI_SR_REF_MASK |
        LPSPI_SR_TEF_MASK |
        LPSPI_SR_TCF_MASK |
        LPSPI_SR_FCF_MASK |
        LPSPI_SR_WCF_MASK |
        LPSPI_SR_RDF_MASK |
        LPSPI_SR_TDF_MASK;

    LPSPI0->TCR =
        LPSPI_TCR_PRESCALE(3) |
        LPSPI_TCR_RXMSK(1) |
        LPSPI_TCR_FRAMESZ(7);

    for (uint32_t i = 0u; i < n; i++)
    {
        timeout = 1000000u;

        while (lpspi0_txfifo_full() && timeout > 0u)
        {
            timeout--;
        }

        if (timeout == 0u)
        {
            return;
        }

        LPSPI0->TDR = tx_buffer[i];
    }

    timeout = 1000000u;

    while (lpspi_busy() && timeout > 0u)
    {
        timeout--;
    }
}

void lpspi_receive(uint8_t *rx_buffer, const uint32_t n)
{
    lpspi_transceive(0, rx_buffer, n);
}

void lpspi_transceive(uint8_t *tx_buffer, uint8_t *rx_buffer, const uint32_t n)
{
    uint32_t timeout;
    uint8_t tx;

    if (rx_buffer == 0 || n == 0u)
    {
        return;
    }

    timeout = 1000000u;

    while (lpspi_busy() && timeout > 0u)
    {
        timeout--;
    }

    if (timeout == 0u)
    {
        return;
    }

    LPSPI0->CR |= LPSPI_CR_RTF(1) | LPSPI_CR_RRF(1);

    LPSPI0->SR =
        LPSPI_SR_MBF_MASK |
        LPSPI_SR_DMF_MASK |
        LPSPI_SR_REF_MASK |
        LPSPI_SR_TEF_MASK |
        LPSPI_SR_TCF_MASK |
        LPSPI_SR_FCF_MASK |
        LPSPI_SR_WCF_MASK |
        LPSPI_SR_RDF_MASK |
        LPSPI_SR_TDF_MASK;

    LPSPI0->TCR =
        LPSPI_TCR_PRESCALE(3) |
        LPSPI_TCR_FRAMESZ(7);

    for (uint32_t i = 0u; i < n; i++)
    {
        timeout = 1000000u;

        while (lpspi0_txfifo_full() && timeout > 0u)
        {
            timeout--;
        }

        if (timeout == 0u)
        {
            return;
        }

        if (tx_buffer == 0)
        {
            tx = dummy_byte;
        }
        else
        {
            tx = tx_buffer[i];
        }

        LPSPI0->TDR = tx;

        timeout = 1000000u;

        while (lpspi0_rxfifo_empty() && timeout > 0u)
        {
            timeout--;
        }

        if (timeout == 0u)
        {
            return;
        }

        rx_buffer[i] = (uint8_t)LPSPI0->RDR;
    }

    timeout = 1000000u;

    while (lpspi_busy() && timeout > 0u)
    {
        timeout--;
    }
}

void lpspi_set_dummy(uint8_t dummy)
{
    dummy_byte = dummy;
}