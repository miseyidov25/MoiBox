#include <MCXA153.h>
#include <stdint.h>

#include "HAL/Audio/buzzer.h"

/*
 * Buzzer pin:
 * P3_5
 */
#define BUZZER_GPIO GPIO3
#define BUZZER_PORT PORT3
#define BUZZER_PIN  5u

#define DOT_MS        150u
#define DASH_MS       450u
#define ELEM_GAP_MS   120u
#define LETTER_GAP_MS 350u
#define WORD_GAP_MS   700u

static void delay_ms_blocking(uint32_t ms)
{
    for (uint32_t m = 0; m < ms; m++)
    {
        for (volatile uint32_t i = 0; i < 6000u; i++)
        {
            __NOP();
        }
    }
}

void buzzer_init(void)
{
    MRCC0->MRCC_GLB_CC1_SET =
        MRCC_MRCC_GLB_CC1_PORT3(1) |
        MRCC_MRCC_GLB_CC1_GPIO3(1);

    MRCC0->MRCC_GLB_RST1_SET =
        MRCC_MRCC_GLB_RST1_PORT3(1) |
        MRCC_MRCC_GLB_RST1_GPIO3(1);

    BUZZER_PORT->PCR[BUZZER_PIN] = PORT_PCR_MUX(0);

    BUZZER_GPIO->PCOR = (1u << BUZZER_PIN);
    BUZZER_GPIO->PDDR |= (1u << BUZZER_PIN);
}

void buzzer_on(void)
{
    BUZZER_GPIO->PSOR = (1u << BUZZER_PIN);
}

void buzzer_off(void)
{
    BUZZER_GPIO->PCOR = (1u << BUZZER_PIN);
}

void buzzer_beep(uint32_t duration_ms)
{
    buzzer_on();
    delay_ms_blocking(duration_ms);
    buzzer_off();
}

void buzzer_dot(void)
{
    buzzer_beep(DOT_MS);
    delay_ms_blocking(ELEM_GAP_MS);
}

void buzzer_dash(void)
{
    buzzer_beep(DASH_MS);
    delay_ms_blocking(ELEM_GAP_MS);
}

static void morse_pattern(const char *pattern)
{
    while (*pattern)
    {
        if (*pattern == '.')
        {
            buzzer_dot();
        }
        else if (*pattern == '-')
        {
            buzzer_dash();
        }

        pattern++;
    }

    delay_ms_blocking(LETTER_GAP_MS);
}

void buzzer_morse_char(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        c = (char)(c - 'a' + 'A');
    }

    switch (c)
    {
        case 'A': morse_pattern(".-"); break;
        case 'B': morse_pattern("-..."); break;
        case 'C': morse_pattern("-.-."); break;
        case 'D': morse_pattern("-.."); break;
        case 'E': morse_pattern("."); break;
        case 'F': morse_pattern("..-."); break;
        case 'G': morse_pattern("--."); break;
        case 'H': morse_pattern("...."); break;
        case 'I': morse_pattern(".."); break;
        case 'J': morse_pattern(".---"); break;
        case 'K': morse_pattern("-.-"); break;
        case 'L': morse_pattern(".-.."); break;
        case 'M': morse_pattern("--"); break;
        case 'N': morse_pattern("-."); break;
        case 'O': morse_pattern("---"); break;
        case 'P': morse_pattern(".--."); break;
        case 'Q': morse_pattern("--.-"); break;
        case 'R': morse_pattern(".-."); break;
        case 'S': morse_pattern("..."); break;
        case 'T': morse_pattern("-"); break;
        case 'U': morse_pattern("..-"); break;
        case 'V': morse_pattern("...-"); break;
        case 'W': morse_pattern(".--"); break;
        case 'X': morse_pattern("-..-"); break;
        case 'Y': morse_pattern("-.--"); break;
        case 'Z': morse_pattern("--.."); break;

        case '0': morse_pattern("-----"); break;
        case '1': morse_pattern(".----"); break;
        case '2': morse_pattern("..---"); break;
        case '3': morse_pattern("...--"); break;
        case '4': morse_pattern("....-"); break;
        case '5': morse_pattern("....."); break;
        case '6': morse_pattern("-...."); break;
        case '7': morse_pattern("--..."); break;
        case '8': morse_pattern("---.."); break;
        case '9': morse_pattern("----."); break;

        case ' ':
            delay_ms_blocking(WORD_GAP_MS);
            break;

        default:
            break;
    }
}

void buzzer_morse_string(const char *text)
{
    while (*text)
    {
        buzzer_morse_char(*text);
        text++;
    }
}

void buzzer_success(void)
{
    buzzer_beep(80u);
    delay_ms_blocking(60u);
    buzzer_beep(80u);
    delay_ms_blocking(60u);
    buzzer_beep(220u);
}

void buzzer_fail(void)
{
    buzzer_beep(300u);
    delay_ms_blocking(100u);
    buzzer_beep(300u);
}

void buzzer_click(void)
{
    buzzer_beep(50u);
}