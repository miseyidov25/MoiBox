#include "HAL/Audio/buzzer.h"

#include <MCXA153.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Active buzzer pin:
 * P3_31 = buzzer signal
 */
#define BUZZER_PIN_MASK     (1u << 31u)

/*
 * Morse timing.
 */
#define DOT_MS              150u
#define DASH_MS             (3u * DOT_MS)
#define ELEMENT_GAP_MS      DOT_MS
#define LETTER_GAP_MS       (3u * DOT_MS)
#define WORD_GAP_MS         (7u * DOT_MS)

/*
 * Rough delay.
 * If sounds are too fast/slow, tune this.
 */
#define DELAY_CALIBRATION   6000u

static bool timed_beep_active = false;
static uint32_t timed_beep_until_ms = 0u;

static void delay_loop(volatile uint32_t count)
{
    while (count > 0u)
    {
        count--;
    }
}

static void delay_ms_blocking(uint32_t duration_ms)
{
    while (duration_ms > 0u)
    {
        delay_loop(DELAY_CALIBRATION);
        duration_ms--;
    }
}

void buzzer_init(void)
{
    timed_beep_active = false;
    timed_beep_until_ms = 0u;

    /*
     * Enable PORT3 and GPIO3.
     */
    MRCC0->MRCC_GLB_CC1_SET =
        MRCC_MRCC_GLB_CC1_PORT3(1) |
        MRCC_MRCC_GLB_CC1_GPIO3(1);

    /*
     * Release PORT3 and GPIO3 from reset.
     */
    MRCC0->MRCC_GLB_RST1_SET =
        MRCC_MRCC_GLB_RST1_PORT3(1) |
        MRCC_MRCC_GLB_RST1_GPIO3(1);

    /*
     * Configure P3_31 as GPIO.
     */
    PORT3->PCR[31] = PORT_PCR_MUX(0);

    /*
     * Set P3_31 as output.
     */
    GPIO3->PDDR |= BUZZER_PIN_MASK;

    buzzer_off();
}

void buzzer_update(uint32_t current_ms)
{
    /*
     * Only timed non-blocking beep.
     * No repeating alarm/reminder anymore.
     */
    if (timed_beep_active && (current_ms >= timed_beep_until_ms))
    {
        buzzer_off();
        timed_beep_active = false;
    }
}

void buzzer_on(void)
{
    /*
     * Active buzzer style:
     * HIGH = sound.
     */
    GPIO3->PSOR = BUZZER_PIN_MASK;
}

void buzzer_off(void)
{
    /*
     * LOW = silent.
     */
    GPIO3->PCOR = BUZZER_PIN_MASK;
}

void buzzer_beep(uint32_t duration_ms)
{
    extern volatile uint32_t ms;

    timed_beep_active = true;
    timed_beep_until_ms = ms + duration_ms;

    buzzer_on();
}

void buzzer_set(uint32_t frequency_hz)
{
    /*
     * Compatibility function.
     *
     * For active buzzer:
     * 0     -> off
     * other -> on
     */
    if (frequency_hz == 0u)
    {
        buzzer_off();
    }
    else
    {
        buzzer_on();
    }
}

static void buzzer_beep_blocking(uint32_t duration_ms)
{
    buzzer_on();
    delay_ms_blocking(duration_ms);
    buzzer_off();
}

static void morse_element_gap(void)
{
    buzzer_off();
    delay_ms_blocking(ELEMENT_GAP_MS);
}

static void morse_letter_gap(void)
{
    buzzer_off();
    delay_ms_blocking(LETTER_GAP_MS);
}

static void morse_word_gap(void)
{
    buzzer_off();
    delay_ms_blocking(WORD_GAP_MS);
}

static void morse_dot(void)
{
    buzzer_beep_blocking(DOT_MS);
}

static void morse_dash(void)
{
    buzzer_beep_blocking(DASH_MS);
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

static void morse_symbol(char symbol)
{
    if (symbol == '.')
    {
        morse_dot();
    }
    else if (symbol == '-')
    {
        morse_dash();
    }
}

void buzzer_morse_digit(uint8_t digit)
{
    static const char *number_morse[10] =
    {
        "-----",
        ".----",
        "..---",
        "...--",
        "....-",
        ".....",
        "-....",
        "--...",
        "---..",
        "----."
    };

    const char *pattern;

    if (digit > 9u)
    {
        digit = digit % 10u;
    }

    pattern = number_morse[digit];

    for (uint8_t i = 0u; i < 5u; i++)
    {
        morse_symbol(pattern[i]);

        if (i < 4u)
        {
            morse_element_gap();
        }
    }

    morse_letter_gap();
}

void buzzer_morse_S(void)
{
    morse_dot();
    morse_element_gap();

    morse_dot();
    morse_element_gap();

    morse_dot();
    morse_letter_gap();
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
            morse_word_gap();
            text++;
            continue;
        }

        const char *pattern = morse_for_char(c);

        while (*pattern != '\0')
        {
            morse_symbol(*pattern);

            pattern++;

            if (*pattern != '\0')
            {
                morse_element_gap();
            }
        }

        morse_letter_gap();
        text++;
    }
}

/*
 * Small victory beeps after puzzle completion.
 */
void buzzer_success(void)
{
    buzzer_repeat_stop();

    buzzer_beep_blocking(70u);
    delay_ms_blocking(70u);

    buzzer_beep_blocking(70u);
    delay_ms_blocking(70u);

    buzzer_beep_blocking(170u);
}

/*
 * Short incorrect beep.
 */
void buzzer_fail(void)
{
    buzzer_repeat_stop();

    buzzer_beep_blocking(120u);
}

void buzzer_click(void)
{
    buzzer_beep_blocking(40u);
}

void buzzer_correct_sound(void)
{
    buzzer_success();
}

void buzzer_error_sound(void)
{
    buzzer_fail();
}

/*
 * Disabled on purpose.
 *
 * Before, this started a repeating beep as soon as a puzzle started.
 * Now the buzzer only sounds for:
 * - Morse code puzzle
 * - short correct/victory beep
 * - short incorrect beep
 */

void buzzer_repeat_start(void)
{
    buzzer_repeat_stop();
}

void buzzer_repeat_stop(void)
{
    timed_beep_active = false;
    timed_beep_until_ms = 0u;

    buzzer_off();
}