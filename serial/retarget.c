/*! ***************************************************************************
 *
 * \brief     Functions for retargeting stdio
 * \file      retarget.c
 * \author    Hugo Arends
 * \date      January 2025
 *
 * \copyright 2025 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include <stdio.h>
#include "serial.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

#ifdef MDKARM

// Functions for redirecting standard output to LPUART for
// the ARM C libraries (used for example in MDK-ARM)

int stdout_putchar(int ch)
{
    serial_putchar(ch);
    return ch;
}

int stdin_getchar(void)
{
    return serial_getchar();
}

#endif

#ifdef __NEWLIB__

// Functions for redirecting standard output to LPUART for
// the Newlib C standard library

int _write(int fd, const void *buf, size_t count)
{
    (void)fd;

    for(size_t i=0; i<count; i++)
    {
        serial_putchar(((char *)buf)[i]);
    }

    return count;
}

int _read(int fd, const void *buf, size_t count)
{
    (void)fd;

    for(size_t i=0; i<count; i++)
    {
        ((char *)buf)[i] = serial_getchar();
    }

    return count;
}

#endif

#ifdef __REDLIB__

// Functions for redirecting standard output to LPUART for
// the Redlib C standard library

int __sys_write(int handle, char *buffer, int size)
{
    if (NULL == buffer)
    {
        // return -1 if error
        return -1;
    }

    // This function only writes to "standard out" and "standard err" for
    // all other file handles it returns failure
    if ((handle != 1) && (handle != 2))
    {
        return -1;
    }

    // Send data
    for(size_t i=0; i<size; i++)
    {
        serial_putchar(((char *)buffer)[i]);
    }

    return size;
}

int __sys_readc(void)
{
    return serial_getchar();
}

#endif
