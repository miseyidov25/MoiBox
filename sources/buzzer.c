#include "buzzer.h"
#include "pins.h"
#include <ctype.h>

// basic morse for A-Z (dots=0,dashes=1 stored in LSB-first pattern)
static const uint8_t morse_map[26] = {
    0x06, // A .-
    0x01, // B -...
    0x02, // C -.-.
    0x03, // D -..
    0x07, // E .
    0x04, // F ..-.
    0x00, // G --.
    0x08, // H ....
    0x0E, // I ..
    0x0E, // J .---
    0x05, // K -.-
    0x02, // L .-..
    0x00, // M --
    0x04, // N -.
    0x00, // O ---
    0x06, // P .--.
    0x00, // Q --.-
    0x05, // R .-.
    0x04, // S ...
    0x00, // T -
    0x0E, // U ..-
    0x0C, // V ...-
    0x04, // W .--
    0x09, // X -..-
    0x01, // Y -.--
    0x02  // Z --..
};

// simple placeholders for hardware control
static void pinMode(uint8_t port, uint8_t pin, uint8_t mode) { (void)port; (void)pin; (void)mode; }
static void digitalWrite(uint8_t port, uint8_t pin, uint8_t val) { (void)port; (void)pin; (void)val; }
static void delay(uint16_t ms) { (void)ms; }

void buzzer_init(void) {
    pinMode(BUZZER_PIN, 1);
    digitalWrite(BUZZER_PIN, 0);
}

void buzzer_play_morse(const char *text) {
    if (!text) return;
    for (int i = 0; text[i]; ++i) {
        char c = text[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        if (c >= 'A' && c <= 'Z') {
            // play pattern (not implemented)
        }
    }
}

// success, error, beep functions would go here as comments
// void buzzer_success(void) { /* ascending tones */ }
// void buzzer_error(void) { /* two buzzes */ }
// etc.

