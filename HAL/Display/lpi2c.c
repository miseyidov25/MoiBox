#include <MCXA153.h>

#include "HAL/Display/lpi2c.h"

#define LPI2C_TIMEOUT   10000U
#define LPI2C_FIFO_SIZE 4U

volatile uint32_t lpi2c_error_step = 0;

static bool s_lpi2c_initialized = false;

static void lpi2c_clear_flags(void)
{
    LPI2C0->MSR = LPI2C_MSR_STF_MASK  |
                  LPI2C_MSR_DMF_MASK  |
                  LPI2C_MSR_PLTF_MASK |
                  LPI2C_MSR_FEF_MASK  |
                  LPI2C_MSR_ALF_MASK  |
                  LPI2C_MSR_NDF_MASK  |
                  LPI2C_MSR_SDF_MASK  |
                  LPI2C_MSR_EPF_MASK;
}

static bool lpi2c_has_error(void)
{
    return ((LPI2C0->MSR & (LPI2C_MSR_NDF_MASK |
                            LPI2C_MSR_ALF_MASK |
                            LPI2C_MSR_FEF_MASK |
                            LPI2C_MSR_PLTF_MASK)) != 0U);
}

bool lpi2c_busy(void)
{
    return ((LPI2C0->MSR & (LPI2C_MSR_BBF_MASK | LPI2C_MSR_MBF_MASK)) != 0U);
}

static bool lpi2c_txfifo_full(void)
{
    uint32_t tx_count;

    tx_count = (LPI2C0->MFSR & LPI2C_MFSR_TXCOUNT_MASK) >> LPI2C_MFSR_TXCOUNT_SHIFT;

    return (tx_count >= LPI2C_FIFO_SIZE);
}

static bool lpi2c_wait_bus_free(void)
{
    uint32_t timeout = LPI2C_TIMEOUT;

    while (lpi2c_busy())
    {
        if (timeout == 0U)
        {
            return false;
        }

        timeout--;
    }

    return true;
}

static bool lpi2c_wait_txfifo_not_full(void)
{
    uint32_t timeout = LPI2C_TIMEOUT;

    while (lpi2c_txfifo_full())
    {
        if (lpi2c_has_error())
        {
            return false;
        }

        if (timeout == 0U)
        {
            return false;
        }

        timeout--;
    }

    return true;
}

static void lpi2c_reset_master(void)
{
    LPI2C0->MCR = 0U;
    LPI2C0->MCR = LPI2C_MCR_RTF(1) | LPI2C_MCR_RRF(1);
    lpi2c_clear_flags();
    LPI2C0->MCR = LPI2C_MCR_MEN(1);
}

static bool lpi2c_send_word(uint32_t command, uint32_t data)
{
    if (!lpi2c_wait_txfifo_not_full())
    {
        return false;
    }

    LPI2C0->MTDR = LPI2C_MTDR_CMD(command) | LPI2C_MTDR_DATA(data);

    return true;
}

void lpi2c_controller_init(void)
{
    if (s_lpi2c_initialized)
    {
        return;
    }

    lpi2c_error_step = 0;

    /*
     * LPI2C0 clock source: FRO_HF_DIV, divider = 1.
     */
    MRCC0->MRCC_LPI2C0_CLKSEL = MRCC_MRCC_LPI2C0_CLKSEL_MUX(2);
    MRCC0->MRCC_LPI2C0_CLKDIV = 0U;

    /*
     * OLED I2C wiring:
     * P3_27 = LPI2C0_SCL
     * P3_28 = LPI2C0_SDA
     */

    /*
     * Enable LPI2C0 and PORT3 clocks.
     */
    MRCC0->MRCC_GLB_CC0_SET =
        MRCC_MRCC_GLB_CC0_LPI2C0(1);

    MRCC0->MRCC_GLB_CC1_SET =
        MRCC_MRCC_GLB_CC1_PORT3(1);

    /*
     * Release LPI2C0 and PORT3 from reset.
     */
    MRCC0->MRCC_GLB_RST0_SET =
        MRCC_MRCC_GLB_RST0_LPI2C0(1);

    MRCC0->MRCC_GLB_RST1_SET =
        MRCC_MRCC_GLB_RST1_PORT3(1);

    /*
     * ALT2, input buffer, open-drain, pull-up.
     * Do NOT lock these pins, so they can still be changed during debugging.
     */
    PORT3->PCR[27] =
        PORT_PCR_IBE(1) |
        PORT_PCR_MUX(2) |
        PORT_PCR_ODE(1) |
        PORT_PCR_PE(1)  |
        PORT_PCR_PS(1);

    PORT3->PCR[28] =
        PORT_PCR_IBE(1) |
        PORT_PCR_MUX(2) |
        PORT_PCR_ODE(1) |
        PORT_PCR_PE(1)  |
        PORT_PCR_PS(1);

    /*
     * Disable master before configuration.
     */
    LPI2C0->MCR = 0U;

    LPI2C0->MCFGR1 = LPI2C_MCFGR1_PRESCALE(0);

    LPI2C0->MCFGR2 =
        LPI2C_MCFGR2_FILTSDA(1) |
        LPI2C_MCFGR2_FILTSCL(1);

    /*
     * Timing values from the OLED branch.
     */
    LPI2C0->MCCR0 =
        LPI2C_MCCR0_DATAVD(0x0F)  |
        LPI2C_MCCR0_SETHOLD(0x1D) |
        LPI2C_MCCR0_CLKHI(0x35)   |
        LPI2C_MCCR0_CLKLO(0x3E);

    LPI2C0->MCR = LPI2C_MCR_RTF(1) | LPI2C_MCR_RRF(1);

    lpi2c_clear_flags();

    LPI2C0->MCR = LPI2C_MCR_MEN(1);

    s_lpi2c_initialized = true;
}

bool lpi2c_init(void)
{
    lpi2c_controller_init();

    return s_lpi2c_initialized;
}

bool lpi2c_write(uint8_t dev_address, uint8_t reg, const uint8_t *p, uint32_t len)
{
    if (!s_lpi2c_initialized)
    {
        return false;
    }

    if ((p == 0) && (len > 0U))
    {
        return false;
    }

    lpi2c_error_step = 0;

    if (!lpi2c_wait_bus_free())
    {
        lpi2c_error_step = 10;
        lpi2c_reset_master();
        return false;
    }

    lpi2c_clear_flags();

    LPI2C0->MCR |= LPI2C_MCR_RTF(1) | LPI2C_MCR_RRF(1);

    /*
     * START + 7-bit device address + write bit.
     */
    if (!lpi2c_send_word(0b100U, ((uint32_t)dev_address << 1)))
    {
        lpi2c_error_step = 20;
        lpi2c_reset_master();
        return false;
    }

    /*
     * OLED control/register byte:
     * 0x00 = command
     * 0x40 = display RAM data
     */
    if (!lpi2c_send_word(0b000U, reg))
    {
        lpi2c_error_step = 30;
        lpi2c_reset_master();
        return false;
    }

    for (uint32_t i = 0U; i < len; i++)
    {
        if (!lpi2c_send_word(0b000U, p[i]))
        {
            lpi2c_error_step = 40;
            lpi2c_reset_master();
            return false;
        }

        if (lpi2c_has_error())
        {
            lpi2c_error_step = 50;
            lpi2c_reset_master();
            return false;
        }
    }

    /*
     * STOP condition.
     */
    if (!lpi2c_send_word(0b010U, 0U))
    {
        lpi2c_error_step = 60;
        lpi2c_reset_master();
        return false;
    }

    if (!lpi2c_wait_bus_free())
    {
        lpi2c_error_step = 70;
        lpi2c_reset_master();
        return false;
    }

    if (lpi2c_has_error())
    {
        lpi2c_error_step = 80;
        lpi2c_reset_master();
        return false;
    }

    return true;
}