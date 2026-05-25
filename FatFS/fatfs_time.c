#include "FatFS/ff.h"

DWORD get_fattime(void)
{
    DWORD year = 2026u - 1980u;
    DWORD month = 5u;
    DWORD day = 22u;
    DWORD hour = 12u;
    DWORD minute = 0u;
    DWORD second = 0u;

    return (year << 25) |
           (month << 21) |
           (day << 16) |
           (hour << 11) |
           (minute << 5) |
           (second / 2u);
}