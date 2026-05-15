#ifndef PUZZLES_HANDLER_H
#define PUZZLES_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

// Puzzle state structures and constants
#define PUZZLE_CODE_LENGTH  7

typedef struct {
    int x;
    int y;
    int z;
    int d;
    int user_answer;
    bool input_active;
    bool solved;
} puzzle1_state_t;

typedef struct {
    char code[PUZZLE_CODE_LENGTH + 1];
    char user_input[PUZZLE_CODE_LENGTH + 1];
    uint8_t input_count;
    bool input_active;
    bool solved;
} puzzle3_state_t;

// Function declarations
void puzzles_init(void);
void puzzles_handle_keypad(void);
void puzzles_handle_serial(void);
void puzzles_process_serial_command(const char *command);
void puzzle_dispatch(const char *minor, int8_t rssi);
void puzzle1_handler(int8_t rssi);
void puzzle2_handler(int8_t rssi);
void puzzle3_handler(int8_t rssi);
void puzzle4_handler(int8_t rssi);
void puzzle5_handler(int8_t rssi);

// Puzzle 1 functions
void puzzle1_process_keypad(char key);
bool is_puzzle1_active(void);

// Puzzle 3 functions
void puzzle3_process_keypad(char key);
bool is_puzzle3_active(void);

#endif // PUZZLES_HANDLER_H
