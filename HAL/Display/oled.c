#include "HAL/Display/oled.h"
#include "HAL/Display/lpi2c.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define OLED_I2C_ADDRESS   0x3C

#define OLED_CTRL_COMMAND  0x00
#define OLED_CTRL_DATA     0x40

#define OLED_WIDTH         256U
#define OLED_HEIGHT        64U
#define OLED_BUFFER_SIZE   ((OLED_WIDTH * OLED_HEIGHT) / 2U)

#define TEXT_BRIGHTNESS    0x0A

static bool s_oled_initialized = false;
static uint8_t framebuffer[OLED_BUFFER_SIZE];

static bool oled_send_command(uint8_t command)
{
    return lpi2c_write(OLED_I2C_ADDRESS, OLED_CTRL_COMMAND, &command, 1U);
}

static bool oled_send_commands(const uint8_t *commands, uint32_t length)
{
    if ((commands == 0) || (length == 0U))
    {
        return false;
    }

    return lpi2c_write(OLED_I2C_ADDRESS, OLED_CTRL_COMMAND, commands, length);
}

static bool oled_send_data_chunk(const uint8_t *data, uint32_t length)
{
    if ((data == 0) || (length == 0U) || (length > 32U))
    {
        return false;
    }

    return lpi2c_write(OLED_I2C_ADDRESS, OLED_CTRL_DATA, data, length);
}

static bool oled_set_full_window(void)
{
    const uint8_t commands[] =
    {
        0x15, 0x00, 0x7F,
        0x75, 0x00, 0x3F
    };

    return oled_send_commands(commands, sizeof(commands));
}

static bool oled_write_ram_command(void)
{
    return oled_send_command(0x5C);
}

static void oled_draw_pixel(uint32_t x, uint32_t y, uint8_t brightness)
{
    uint32_t index;

    if ((x >= OLED_WIDTH) || (y >= OLED_HEIGHT))
    {
        return;
    }

    brightness &= 0x0F;

    index = ((y * OLED_WIDTH) + x) / 2U;

    if ((x % 2U) == 0U)
    {
        framebuffer[index] = (framebuffer[index] & 0x0FU) | (brightness << 4);
    }
    else
    {
        framebuffer[index] = (framebuffer[index] & 0xF0U) | brightness;
    }
}

static void oled_draw_scaled_pixel(uint32_t x, uint32_t y, uint32_t scale)
{
    uint32_t draw_size = scale;

    if (scale == 0U)
    {
        return;
    }

    if (scale > 1U)
    {
        draw_size = scale - 1U;
    }

    for (uint32_t yy = 0U; yy < draw_size; yy++)
    {
        for (uint32_t xx = 0U; xx < draw_size; xx++)
        {
            oled_draw_pixel(x + xx, y + yy, TEXT_BRIGHTNESS);
        }
    }
}

static char oled_to_upper(char c)
{
    if ((c >= 'a') && (c <= 'z'))
    {
        return (char)(c - 'a' + 'A');
    }

    return c;
}

static const uint8_t *font_get_char(char c)
{
    static const uint8_t blank[5] = {0, 0, 0, 0, 0};

    static const uint8_t C_A[5] = {0x7E, 0x09, 0x09, 0x09, 0x7E};
    static const uint8_t C_B[5] = {0x7F, 0x49, 0x49, 0x49, 0x36};
    static const uint8_t C_C[5] = {0x3E, 0x41, 0x41, 0x41, 0x22};
    static const uint8_t C_D[5] = {0x7F, 0x41, 0x41, 0x22, 0x1C};
    static const uint8_t C_E[5] = {0x7F, 0x49, 0x49, 0x49, 0x41};
    static const uint8_t C_F[5] = {0x7F, 0x09, 0x09, 0x09, 0x01};
    static const uint8_t C_G[5] = {0x3E, 0x41, 0x49, 0x49, 0x7A};
    static const uint8_t C_H[5] = {0x7F, 0x08, 0x08, 0x08, 0x7F};
    static const uint8_t C_I[5] = {0x00, 0x41, 0x7F, 0x41, 0x00};
    static const uint8_t C_J[5] = {0x20, 0x40, 0x41, 0x3F, 0x01};
    static const uint8_t C_K[5] = {0x7F, 0x08, 0x14, 0x22, 0x41};
    static const uint8_t C_L[5] = {0x7F, 0x40, 0x40, 0x40, 0x40};
    static const uint8_t C_M[5] = {0x7F, 0x02, 0x04, 0x02, 0x7F};
    static const uint8_t C_N[5] = {0x7F, 0x04, 0x08, 0x10, 0x7F};
    static const uint8_t C_O[5] = {0x3E, 0x41, 0x41, 0x41, 0x3E};
    static const uint8_t C_P[5] = {0x7F, 0x09, 0x09, 0x09, 0x06};
    static const uint8_t C_Q[5] = {0x3E, 0x41, 0x51, 0x21, 0x5E};
    static const uint8_t C_R[5] = {0x7F, 0x09, 0x19, 0x29, 0x46};
    static const uint8_t C_S[5] = {0x46, 0x49, 0x49, 0x49, 0x31};
    static const uint8_t C_T[5] = {0x01, 0x01, 0x7F, 0x01, 0x01};
    static const uint8_t C_U[5] = {0x3F, 0x40, 0x40, 0x40, 0x3F};
    static const uint8_t C_V[5] = {0x1F, 0x20, 0x40, 0x20, 0x1F};
    static const uint8_t C_W[5] = {0x7F, 0x20, 0x18, 0x20, 0x7F};
    static const uint8_t C_X[5] = {0x63, 0x14, 0x08, 0x14, 0x63};
    static const uint8_t C_Y[5] = {0x07, 0x08, 0x70, 0x08, 0x07};
    static const uint8_t C_Z[5] = {0x61, 0x51, 0x49, 0x45, 0x43};

    static const uint8_t C_0[5] = {0x3E, 0x51, 0x49, 0x45, 0x3E};
    static const uint8_t C_1[5] = {0x00, 0x42, 0x7F, 0x40, 0x00};
    static const uint8_t C_2[5] = {0x62, 0x51, 0x49, 0x49, 0x46};
    static const uint8_t C_3[5] = {0x22, 0x41, 0x49, 0x49, 0x36};
    static const uint8_t C_4[5] = {0x18, 0x14, 0x12, 0x7F, 0x10};
    static const uint8_t C_5[5] = {0x27, 0x45, 0x45, 0x45, 0x39};
    static const uint8_t C_6[5] = {0x3C, 0x4A, 0x49, 0x49, 0x30};
    static const uint8_t C_7[5] = {0x01, 0x71, 0x09, 0x05, 0x03};
    static const uint8_t C_8[5] = {0x36, 0x49, 0x49, 0x49, 0x36};
    static const uint8_t C_9[5] = {0x06, 0x49, 0x49, 0x29, 0x1E};

    static const uint8_t C_PLUS[5]     = {0x08, 0x08, 0x3E, 0x08, 0x08};
    static const uint8_t C_MINUS[5]    = {0x08, 0x08, 0x08, 0x08, 0x08};
    static const uint8_t C_STAR[5]     = {0x22, 0x14, 0x08, 0x14, 0x22};
    static const uint8_t C_EQUALS[5]   = {0x14, 0x14, 0x14, 0x14, 0x14};
    static const uint8_t C_QUESTION[5] = {0x02, 0x01, 0x51, 0x09, 0x06};
    static const uint8_t C_COLON[5]    = {0x00, 0x36, 0x36, 0x00, 0x00};
    static const uint8_t C_DOT[5]      = {0x00, 0x60, 0x60, 0x00, 0x00};
    static const uint8_t C_COMMA[5]    = {0x00, 0x80, 0x60, 0x00, 0x00};
    static const uint8_t C_SLASH[5]    = {0x20, 0x10, 0x08, 0x04, 0x02};
    static const uint8_t C_HASH[5]     = {0x14, 0x7F, 0x14, 0x7F, 0x14};

    c = oled_to_upper(c);

    switch (c)
    {
        case 'A': return C_A;
        case 'B': return C_B;
        case 'C': return C_C;
        case 'D': return C_D;
        case 'E': return C_E;
        case 'F': return C_F;
        case 'G': return C_G;
        case 'H': return C_H;
        case 'I': return C_I;
        case 'J': return C_J;
        case 'K': return C_K;
        case 'L': return C_L;
        case 'M': return C_M;
        case 'N': return C_N;
        case 'O': return C_O;
        case 'P': return C_P;
        case 'Q': return C_Q;
        case 'R': return C_R;
        case 'S': return C_S;
        case 'T': return C_T;
        case 'U': return C_U;
        case 'V': return C_V;
        case 'W': return C_W;
        case 'X': return C_X;
        case 'Y': return C_Y;
        case 'Z': return C_Z;

        case '0': return C_0;
        case '1': return C_1;
        case '2': return C_2;
        case '3': return C_3;
        case '4': return C_4;
        case '5': return C_5;
        case '6': return C_6;
        case '7': return C_7;
        case '8': return C_8;
        case '9': return C_9;

        case '+': return C_PLUS;
        case '-': return C_MINUS;
        case '*': return C_STAR;
        case '=': return C_EQUALS;
        case '?': return C_QUESTION;
        case ':': return C_COLON;
        case '.': return C_DOT;
        case ',': return C_COMMA;
        case '/': return C_SLASH;
        case '#': return C_HASH;
        case ' ': return blank;

        default:
            return blank;
    }
}

static uint32_t oled_string_length(const char *text)
{
    uint32_t length = 0U;

    if (text == 0)
    {
        return 0U;
    }

    while (text[length] != '\0')
    {
        length++;
    }

    return length;
}

static void oled_draw_char(uint32_t x, uint32_t y, char c, uint32_t scale)
{
    const uint8_t *pattern;

    if (scale == 0U)
    {
        return;
    }

    pattern = font_get_char(c);

    for (uint32_t col = 0U; col < 5U; col++)
    {
        uint8_t bits = pattern[col];

        for (uint32_t row = 0U; row < 7U; row++)
        {
            if ((bits & (1U << row)) != 0U)
            {
                oled_draw_scaled_pixel(x + (col * scale), y + (row * scale), scale);
            }
        }
    }
}

static void oled_draw_string(uint32_t x, uint32_t y, const char *text, uint32_t scale)
{
    uint32_t cursor_x = x;

    if (text == 0)
    {
        return;
    }

    while (*text != '\0')
    {
        oled_draw_char(cursor_x, y, *text, scale);
        cursor_x += 6U * scale;
        text++;
    }
}

static void oled_draw_string_centered(uint32_t y, const char *text, uint32_t scale)
{
    uint32_t length;
    uint32_t width = 0U;
    uint32_t x = 0U;

    if (text == 0)
    {
        return;
    }

    length = oled_string_length(text);

    if (length > 0U)
    {
        width = (length * 6U * scale) - scale;
    }

    if (width < OLED_WIDTH)
    {
        x = (OLED_WIDTH - width) / 2U;
    }

    oled_draw_string(x, y, text, scale);
}

static void oled_startup_delay(void)
{
    volatile uint32_t i;

    for (i = 0u; i < 3000000u; i++)
    {
        __asm volatile ("nop");
    }
}

void oled_draw_text(uint32_t x, uint32_t y, const char *text, uint32_t scale)
{
    oled_draw_string(x, y, text, scale);
}

void oled_draw_text_centered(uint32_t y, const char *text, uint32_t scale)
{
    oled_draw_string_centered(y, text, scale);
}

bool oled_init(void)
{
    oled_startup_delay();
    
    if (!lpi2c_init())
    {
        s_oled_initialized = false;
        return false;
    }

    const uint8_t init_sequence[] =
    {
        0xFD, 0x12,
        0xAE,

        0x15, 0x00, 0x7F,
        0x75, 0x00, 0x3F,

        0x81, 0x55,

        0xA0, 0xC1,
        0xA1, 0x00,
        0xA2, 0x00,
        0xA4,
        0xA8, 0x3F,
        0xAB, 0x01,
        0xAD, 0x8E,
        0xB1, 0x22,
        0xB3, 0xA0,
        0xB6, 0x04,
        0xB9,
        0xBC, 0x10,
        0xBD, 0x01,
        0xBE, 0x07
    };

    if (!oled_send_commands(init_sequence, sizeof(init_sequence)))
    {
        s_oled_initialized = false;
        return false;
    }

    s_oled_initialized = true;

    oled_clear();

    if (!oled_send_command(0xAF))
    {
        s_oled_initialized = false;
        return false;
    }

    oled_update();

    return true;
}

void oled_clear(void)
{
    for (uint32_t i = 0U; i < OLED_BUFFER_SIZE; i++)
    {
        framebuffer[i] = 0x00;
    }

    /*
     * Auto-update so existing project calls still work.
     */
    oled_update();
}

void oled_update(void)
{
    uint32_t remaining;
    uint32_t index;

    if (!s_oled_initialized)
    {
        return;
    }

    if (!oled_set_full_window())
    {
        return;
    }

    if (!oled_write_ram_command())
    {
        return;
    }

    remaining = OLED_BUFFER_SIZE;
    index = 0U;

    while (remaining > 0U)
    {
        uint32_t chunk = remaining;

        if (chunk > 32U)
        {
            chunk = 32U;
        }

        if (!oled_send_data_chunk(&framebuffer[index], chunk))
        {
            return;
        }

        index += chunk;
        remaining -= chunk;
    }
}

void oled_display_string(uint8_t row, uint8_t col, const char *str)
{
    uint32_t x;
    uint32_t y;

    if (str == 0)
    {
        return;
    }

    x = ((uint32_t)col) * 12U;
    y = ((uint32_t)row) * 16U;

    oled_draw_string(x, y, str, 2U);
    oled_update();
}

void oled_display_value(uint8_t row, uint8_t col, int32_t value)
{
    char buffer[16];

    snprintf(buffer, sizeof(buffer), "%ld", (long)value);

    oled_display_string(row, col, buffer);
}

void oled_display_puzzle_state(const char *puzzle_name, const char *state)
{
    oled_clear();

    if (puzzle_name != 0)
    {
        oled_display_string(0U, 0U, puzzle_name);
    }

    if (state != 0)
    {
        oled_display_string(1U, 0U, state);
    }

    oled_update();
}
