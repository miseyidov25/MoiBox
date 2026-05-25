#include <MCXA153.h>

#include "FatFS/ff.h"
#include "FatFS/diskio.h"
#include "FatFS/lpspi_master.h"

#define SD_CS_PIN 3u

static void sd_cs_init(void)
{
    MRCC0->MRCC_GLB_CC0_SET =
        MRCC_MRCC_GLB_CC0_PORT1(1);

    MRCC0->MRCC_GLB_CC1_SET =
        MRCC_MRCC_GLB_CC1_GPIO1(1);

    MRCC0->MRCC_GLB_RST0_SET =
        MRCC_MRCC_GLB_RST0_PORT1(1);

    MRCC0->MRCC_GLB_RST1_SET =
        MRCC_MRCC_GLB_RST1_GPIO1(1);

    PORT1->PCR[SD_CS_PIN] = PORT_PCR_MUX(0);

    GPIO1->PDDR |= (1u << SD_CS_PIN);
    GPIO1->PSOR = (1u << SD_CS_PIN);
}

#define CS_INIT()   sd_cs_init()
#define CS_H()      (GPIO1->PSOR = (1u << SD_CS_PIN))
#define CS_L()      (GPIO1->PCOR = (1u << SD_CS_PIN))

extern volatile uint32_t ms;

static void dly_ms(UINT n)
{
    uint32_t timeout = ms + n;

    while (ms < timeout)
    {
    }
}

#define CMD0     (0)
#define CMD1     (1)
#define ACMD41   (0x80 + 41)
#define CMD8     (8)
#define CMD9     (9)
#define CMD10    (10)
#define CMD12    (12)
#define CMD13    (13)
#define ACMD13   (0x80 + 13)
#define CMD16    (16)
#define CMD17    (17)
#define CMD18    (18)
#define CMD23    (23)
#define ACMD23   (0x80 + 23)
#define CMD24    (24)
#define CMD25    (25)
#define CMD32    (32)
#define CMD33    (33)
#define CMD38    (38)
#define CMD55    (55)
#define CMD58    (58)

static DSTATUS Stat = STA_NOINIT;
static BYTE CardType;

static void xmit_mmc(const BYTE *buff, UINT bc)
{
    lpspi_transmit((uint8_t *)buff, bc);
}

static void rcvr_mmc(BYTE *buff, UINT bc)
{
    lpspi_receive(buff, bc);
}

static int wait_ready(void)
{
    BYTE d;
    UINT tmr;

    for (tmr = 500; tmr; tmr--)
    {
        rcvr_mmc(&d, 1);

        if (d == 0xFF)
        {
            break;
        }

        dly_ms(1);
    }

    return tmr ? 1 : 0;
}

static void deselect(void)
{
    BYTE d;

    CS_H();
    rcvr_mmc(&d, 1);
}

static int select_card(void)
{
    BYTE d;

    CS_L();
    rcvr_mmc(&d, 1);

    if (wait_ready())
    {
        return 1;
    }

    deselect();
    return 0;
}

static int rcvr_datablock(BYTE *buff, UINT btr)
{
    BYTE d[2];
    UINT tmr;

    for (tmr = 100; tmr; tmr--)
    {
        rcvr_mmc(d, 1);

        if (d[0] != 0xFF)
        {
            break;
        }

        dly_ms(1);
    }

    if (d[0] != 0xFE)
    {
        return 0;
    }

    rcvr_mmc(buff, btr);
    rcvr_mmc(d, 2);

    return 1;
}

static int xmit_datablock(const BYTE *buff, BYTE token)
{
    BYTE d[2];

    if (!wait_ready())
    {
        return 0;
    }

    d[0] = token;
    xmit_mmc(d, 1);

    if (token != 0xFD)
    {
        xmit_mmc(buff, 512);

        d[0] = 0xFF;
        d[1] = 0xFF;
        xmit_mmc(d, 2);

        rcvr_mmc(d, 1);

        if ((d[0] & 0x1F) != 0x05)
        {
            return 0;
        }
    }

    return 1;
}

static BYTE send_cmd_raw(BYTE cmd, DWORD arg)
{
    BYTE n;
    BYTE d;
    BYTE buf[6];

    deselect();

    if (!select_card())
    {
        return 0xFF;
    }

    buf[0] = 0x40 | cmd;
    buf[1] = (BYTE)(arg >> 24);
    buf[2] = (BYTE)(arg >> 16);
    buf[3] = (BYTE)(arg >> 8);
    buf[4] = (BYTE)arg;

    n = 0x01;

    if (cmd == CMD0)
    {
        n = 0x95;
    }

    if (cmd == CMD8)
    {
        n = 0x87;
    }

    buf[5] = n;

    xmit_mmc(buf, 6);

    if (cmd == CMD12)
    {
        rcvr_mmc(&d, 1);
    }

    n = 10;

    do
    {
        rcvr_mmc(&d, 1);
    }
    while ((d & 0x80) && --n);

    return d;
}

static BYTE send_cmd(BYTE cmd, DWORD arg)
{
    BYTE r;

    if (cmd & 0x80)
    {
        cmd &= 0x7F;

        r = send_cmd_raw(CMD55, 0);

        if (r > 1)
        {
            return r;
        }

        deselect();

        return send_cmd_raw(cmd, arg);
    }

    return send_cmd_raw(cmd, arg);
}

DSTATUS disk_status(BYTE drv)
{
    if (drv)
    {
        return STA_NOINIT;
    }

    return Stat;
}

DSTATUS disk_initialize(BYTE drv)
{
    BYTE n;
    BYTE ty;
    BYTE cmd;
    BYTE buf[4];
    UINT tmr;
    DSTATUS s;
    BYTE r;

    if (drv)
    {
        return STA_NOINIT;
    }

    Stat = STA_NOINIT;
    CardType = 0;

    dly_ms(20);

    lpspi_master_init();
    lpspi_set_dummy(0xFF);

    CS_INIT();
    CS_H();

    dly_ms(20);

    for (n = 10; n; n--)
    {
        rcvr_mmc(buf, 1);
    }

    ty = 0;

    r = send_cmd(CMD0, 0);

    if (r == 1)
    {
        r = send_cmd(CMD8, 0x1AA);

        if (r == 1)
        {
            rcvr_mmc(buf, 4);

            if (buf[2] == 0x01 && buf[3] == 0xAA)
            {
                r = send_cmd(CMD58, 0);

                if (r == 0 || r == 1)
                {
                    rcvr_mmc(buf, 4);
                }

                for (tmr = 10000; tmr; tmr--)
                {
                    r = send_cmd(ACMD41, 1UL << 30);

                    if (r == 0)
                    {
                        break;
                    }

                    dly_ms(1);
                }

                if (tmr)
                {
                    r = send_cmd(CMD58, 0);

                    if (r == 0)
                    {
                        rcvr_mmc(buf, 4);
                        ty = (buf[0] & 0x40) ? (CT_SDC2 | CT_BLOCK) : CT_SDC2;
                    }
                }
                else
                {
                    /*
                     * Fallback for unusual cards.
                     */
                    for (tmr = 5000; tmr; tmr--)
                    {
                        r = send_cmd(CMD1, 0);

                        if (r == 0)
                        {
                            break;
                        }

                        dly_ms(1);
                    }

                    if (tmr)
                    {
                        r = send_cmd(CMD16, 512);

                        if (r == 0)
                        {
                            ty = CT_MMC3;
                        }
                    }
                }
            }
        }
        else
        {
            r = send_cmd(ACMD41, 0);

            if (r <= 1)
            {
                ty = CT_SDC2;
                cmd = ACMD41;
            }
            else
            {
                ty = CT_MMC3;
                cmd = CMD1;
            }

            for (tmr = 1000; tmr; tmr--)
            {
                r = send_cmd(cmd, 0);

                if (r == 0)
                {
                    break;
                }

                dly_ms(1);
            }

            if (!tmr || send_cmd(CMD16, 512) != 0)
            {
                ty = 0;
            }
        }
    }

    CardType = ty;
    s = ty ? 0 : STA_NOINIT;
    Stat = s;

    deselect();

    return s;
}

DRESULT disk_read(BYTE drv, BYTE *buff, LBA_t sector, UINT count)
{
    BYTE cmd;
    DWORD sect = (DWORD)sector;

    if (disk_status(drv) & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    if (!(CardType & CT_BLOCK))
    {
        sect *= 512;
    }

    cmd = count > 1 ? CMD18 : CMD17;

    if (send_cmd(cmd, sect) == 0)
    {
        do
        {
            if (!rcvr_datablock(buff, 512))
            {
                break;
            }

            buff += 512;
        }
        while (--count);

        if (cmd == CMD18)
        {
            send_cmd(CMD12, 0);
        }
    }

    deselect();

    return count ? RES_ERROR : RES_OK;
}

DRESULT disk_write(BYTE drv, const BYTE *buff, LBA_t sector, UINT count)
{
    DWORD sect = (DWORD)sector;

    if (disk_status(drv) & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    if (!(CardType & CT_BLOCK))
    {
        sect *= 512;
    }

    if (count == 1)
    {
        if ((send_cmd(CMD24, sect) == 0) &&
            xmit_datablock(buff, 0xFE))
        {
            count = 0;
        }
    }
    else
    {
        if (CardType & CT_SDC)
        {
            send_cmd(ACMD23, count);
        }

        if (send_cmd(CMD25, sect) == 0)
        {
            do
            {
                if (!xmit_datablock(buff, 0xFC))
                {
                    break;
                }

                buff += 512;
            }
            while (--count);

            if (!xmit_datablock(0, 0xFD))
            {
                count = 1;
            }
        }
    }

    deselect();

    return count ? RES_ERROR : RES_OK;
}

DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
    DRESULT res;
    BYTE csd[16];
    DWORD cs;
    BYTE n;

    if (disk_status(drv) & STA_NOINIT)
    {
        return RES_NOTRDY;
    }

    res = RES_ERROR;

    switch (ctrl)
    {
        case CTRL_SYNC:
            if (select_card())
            {
                res = RES_OK;
            }
            break;

        case GET_SECTOR_COUNT:
            if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
            {
                if ((csd[0] >> 6) == 1)
                {
                    cs = csd[9] +
                         ((WORD)csd[8] << 8) +
                         ((DWORD)(csd[7] & 63) << 16) +
                         1;

                    *(LBA_t *)buff = cs << 10;
                }
                else
                {
                    n = (csd[5] & 15) +
                        ((csd[10] & 128) >> 7) +
                        ((csd[9] & 3) << 1) +
                        2;

                    cs = (csd[8] >> 6) +
                         ((WORD)csd[7] << 2) +
                         ((WORD)(csd[6] & 3) << 10) +
                         1;

                    *(LBA_t *)buff = cs << (n - 9);
                }

                res = RES_OK;
            }
            break;

        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 128;
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
            break;
    }

    deselect();

    return res;
}