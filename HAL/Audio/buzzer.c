#include "HAL/Audio/buzzer.h"

#include <MCXA153.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Set to 1 when buzzer pin is confirmed.
 *
 * While this is 0, the code compiles and puzzle logic works,
 * but no real sound comes out.
 */
#define BUZZER_ENABLED 0

/*
 * Change to match the actual GPIO pin used for the buzzer.
 * The current settings are for P3_5, which is the default on the A153
 */
#define BUZZER_GPIO GPIO3
#define BUZZER_PORT PORT3
#define BUZZER_PIN  5u

static bool buzzer_active = false;
static uint32_t buzzer_until_ms = 0u;

static volatile uint32_t local_delay_counter;

static void tiny_delay(uint32_t count)
{
    local_delay_counter = count;

    while (local_delay_counter > 0u)
    {
        local_delay_counter--;
    }
}

static void pause_short(void)
{
    tiny_delay(35000u);
}

static void pause_medium(void)
{
    tiny_delay(90000u);
}

static void pause_long(void)
{
    tiny_delay(180000u);
}

static void buzzer_pin_high(void)
{
#if BUZZER_ENABLED
    BUZZER_GPIO->PSOR = (1u << BUZZER_PIN);
#endif
}

static void buzzer_pin_low(void)
{
#if BUZZER_ENABLED
    BUZZER_GPIO->PCOR = (1u << BUZZER_PIN);
#endif
}

static void tone_blocking(uint32_t cycles, uint32_t half_period_delay)
{
#if BUZZER_ENABLED
    for (uint32_t i = 0u; i < cycles; i++)
    {
        buzzer_pin_high();
        tiny_delay(half_period_delay);
        buzzer_pin_low();
        tiny_delay(half_period_delay);
    }
#else
    (void)cycles;
    (void)half_period_delay;
#endif
}

static void morse_dot(void)
{
    tone_blocking(70u, 500u);
    pause_short();
}

static void morse_dash(void)
{
    tone_blocking(210u, 500u);
    pause_short();
}

static void morse_gap_letter(void)
{
    pause_medium();
}

static void morse_gap_word(void)
{
    pause_long();
}

static char to_upper(char c)
{
    if ((c >= 'a') && (c <= 'z'))
    {
        return (char)(c - 'a' + 'A');
    }

    return c;
}

static const char *morse_for_char(char c)
{
    c = to_upper(c);

    switch (c)
    {
        case 'A': return ".-";
        case 'B': return "-...";
        case 'C': return "-.-.";
        case 'D': return "-..";
        case 'E': return ".";
        case 'F': return "..-.";
        case 'G': return "--.";
        case 'H': return "....";
        case 'I': return "..";
        case 'J': return ".---";
        case 'K': return "-.-";
        case 'L': return ".-..";
        case 'M': return "--";
        case 'N': return "-.";
        case 'O': return "---";
        case 'P': return ".--.";
        case 'Q': return "--.-";
        case 'R': return ".-.";
        case 'S': return "...";
        case 'T': return "-";
        case 'U': return "..-";
        case 'V': return "...-";
        case 'W': return ".--";
        case 'X': return "-..-";
        case 'Y': return "-.--";
        case 'Z': return "--..";

        case '0': return "-----";
        case '1': return ".----";
        case '2': return "..---";
        case '3': return "...--";
        case '4': return "....-";
        case '5': return ".....";
        case '6': return "-....";
        case '7': return "--...";
        case '8': return "---..";
        case '9': return "----.";

        default:
            return "";
    }
}

void buzzer_init(void)
{
    buzzer_active = false;
    buzzer_until_ms = 0u;

#if BUZZER_ENABLED
    /*
     * Enable PORT3/GPIO3 if using P3_x.
     * Change this if buzzer uses another port.
     */
    MRCC0->MRCC_GLB_CC1_SET |= MRCC_MRCC_GLB_CC1_PORT3(1);
    MRCC0->MRCC_GLB_CC1_SET |= MRCC_MRCC_GLB_CC1_GPIO3(1);

    MRCC0->MRCC_GLB_RST1_SET |= MRCC_MRCC_GLB_RST1_PORT3(1);
    MRCC0->MRCC_GLB_RST1_SET |= MRCC_MRCC_GLB_RST1_GPIO3(1);

    BUZZER_PORT->PCR[BUZZER_PIN] =
        PORT_PCR_MUX(0) |
        PORT_PCR_IBE(1);

    BUZZER_GPIO->PDDR |= (1u << BUZZER_PIN);
    buzzer_pin_low();
#endif
}

void buzzer_update(uint32_t current_ms)
{
    if (buzzer_active && (current_ms >= buzzer_until_ms))
    {
        buzzer_off();
    }
}

void buzzer_on(void)
{
    buzzer_active = true;
    buzzer_pin_high();
}

void buzzer_off(void)
{
    buzzer_active = false;
    buzzer_pin_low();
}

void buzzer_beep(uint32_t duration_ms)
{
    extern volatile uint32_t ms;

    buzzer_active = true;
    buzzer_until_ms = ms + duration_ms;
    buzzer_pin_high();
}

void buzzer_correct_sound(void)
{

    tone_blocking(80u, 600u);
    pause_short();
    tone_blocking(120u, 350u);
}

void buzzer_error_sound(void)
{

    tone_blocking(180u, 1000u);
}

void buzzer_success(void)
{
    buzzer_correct_sound();
}

void buzzer_fail(void)
{
    buzzer_error_sound();
}

void buzzer_morse_string(const char *text)
{
    if (text == 0)
    {
        return;
    }

    while (*text != '\0')
    {
        char c = *text;

        if (c == ' ')
        {
            morse_gap_word();
            text++;
            continue;
        }

        const char *pattern = morse_for_char(c);

        while (*pattern != '\0')
        {
            if (*pattern == '.')
            {
                morse_dot();
            }
            else if (*pattern == '-')
            {
                morse_dash();
            }

            pattern++;
        }

        morse_gap_letter();
        text++;
    }
}