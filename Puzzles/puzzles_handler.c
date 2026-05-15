#include "puzzles_handler.h"
#include "../HAL/Input/keypad.h"
#include "../serial/serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Puzzle state variables
static puzzle1_state_t puzzle1;
static bool puzzle2_active = false;
static puzzle3_state_t puzzle3;
static bool puzzle4_active = false;
static bool puzzle5_active = false;

// Serial input buffer
#define SERIAL_BUFFER_SIZE 32
static char serial_buffer[SERIAL_BUFFER_SIZE];
static uint8_t serial_buffer_index = 0;

//Initialize puzzle system
void puzzles_init(void)
{
    memset(&puzzle1, 0, sizeof(puzzle1_state_t));
    memset(&puzzle3, 0, sizeof(puzzle3_state_t));
    puzzle2_active = false;
    puzzle4_active = false;
    puzzle5_active = false;
}

// Handle puzzle keypad input in main loop
void puzzles_handle_keypad(void)
{
    char keypad_key = '\0';
    if (keypad_getkey(&keypad_key))
    {
        if (is_puzzle1_active())
        {
            puzzle1_process_keypad(keypad_key);
        }
        else if (is_puzzle3_active())
        {
            puzzle3_process_keypad(keypad_key);
        }
    }
}

// Handle serial input for puzzles
void puzzles_handle_serial(void)
{
    while (serial_rxcnt() > 0)
    {
        int ch = serial_getchar();
        if (ch != -1)
        {
            char c = (char)ch;
            
            // Handle backspace
            if (c == '\b' || c == 127)
            {
                if (serial_buffer_index > 0)
                {
                    serial_buffer_index--;
                    serial_putchar('\b');
                    serial_putchar(' ');
                    serial_putchar('\b');
                }
                continue;
            }
            
            // Handle enter/newline
            if (c == '\r' || c == '\n')
            {
                serial_buffer[serial_buffer_index] = '\0';
                
                if (serial_buffer_index > 0)
                {
                    puzzles_process_serial_command(serial_buffer);
                }
                
                serial_buffer_index = 0;
                printf("\r\n> ");
                continue;
            }
            
            // Add character to buffer if space available
            if (serial_buffer_index < SERIAL_BUFFER_SIZE - 1)
            {
                serial_buffer[serial_buffer_index++] = c;
                serial_putchar(c);
            }
        }
    }
}

// Process serial commands
void puzzles_process_serial_command(const char *command)
{
    // Handle puzzle start commands
    if (strcmp(command, "Puzzle1") == 0 || strcmp(command, "puzzle1") == 0)
    {
        puzzle1_handler(0);
    }
    else if (strcmp(command, "Puzzle2") == 0 || strcmp(command, "puzzle2") == 0)
    {
        puzzle2_handler(0);
    }
    else if (strcmp(command, "Puzzle3") == 0 || strcmp(command, "puzzle3") == 0)
    {
        puzzle3_handler(0);
    }
    else if (strcmp(command, "Puzzle4") == 0 || strcmp(command, "puzzle4") == 0)
    {
        puzzle4_handler(0);
    }
    else if (strcmp(command, "Puzzle5") == 0 || strcmp(command, "puzzle5") == 0)
    {
        puzzle5_handler(0);
    }
    else
    {
        // Handle numeric input for active puzzles
        if (is_puzzle1_active() && strlen(command) == 1 && command[0] >= '0' && command[0] <= '9')
        {
            puzzle1_process_keypad(command[0]);
        }
        else if (is_puzzle3_active() && strlen(command) == 1 && command[0] >= '0' && command[0] <= '9')
        {
            puzzle3_process_keypad(command[0]);
        }
        else
        {
            printf("\r\nUnknown command: %s\r\n", command);
            printf("Available commands: Puzzle1, Puzzle2, Puzzle3, Puzzle4, Puzzle5\r\n");
            printf("Or enter numbers 0-9 for active puzzles\r\n");
        }
    }
}

//Dispatch beacon to appropriate puzzle handler
void puzzle_dispatch(const char *minor, int8_t rssi)
{
    if (strcmp(minor, "0037") == 0)
    {
        puzzle1_handler(rssi);
    }
    else if (strcmp(minor, "0032") == 0)
    {
        puzzle2_handler(rssi);
    }
    else if (strcmp(minor, "0026") == 0)
    {
        puzzle3_handler(rssi);
    }
    else if (strcmp(minor, "0029") == 0)
    {
        puzzle4_handler(rssi);
    }
    else if (strcmp(minor, "0031") == 0)
    {
        puzzle5_handler(rssi);
    }
}

// PUZZLE 1: Math Challenge
// Display: x + y = z, y + y = d
// Task: Solve for X using keypad input (0-9)
void puzzle1_handler(int8_t rssi)
{
    puzzle1.solved = false;
    puzzle1.input_active = true;
    
    // Generate random numbers for the puzzle
    puzzle1.x = (rand() % 10) + 1;      // x: 1-10
    puzzle1.y = (rand() % 10) + 1;      // y: 1-10
    puzzle1.z = puzzle1.x + puzzle1.y;  // z = x + y
    puzzle1.d = puzzle1.y + puzzle1.y;  // d = y + y
    puzzle1.user_answer = 0;
    
    printf("\r\n========== PUZZLE 1: MATH CHALLENGE ==========\r\n");
    printf("%d + %d = %d\r\n", puzzle1.x, puzzle1.y, puzzle1.z);
    printf("%d + %d = %d\r\n", puzzle1.y, puzzle1.y, puzzle1.d);
    printf("\r\n");
    printf("Find X using the keypad (press 0-9):\r\n");
    printf("Your answer: ");
}

void puzzle1_process_keypad(char key)
{
    if (!puzzle1.input_active)
    {
        return;
    }

    if (key >= '0' && key <= '9')
    {
        int user_digit = key - '0';
        puzzle1.user_answer = user_digit;
        printf("%c\r\n", key);
        
        if (puzzle1.user_answer == puzzle1.x)
        {
            printf("\r\n========== CORRECT! ==========\r\n");
            printf("X = %d is the correct answer!\r\n", puzzle1.x);
            printf("PUZZLE 1 SOLVED!\r\n");
            puzzle1.solved = true;
        }
        else
        {
            printf("\r\nWrong! Try again.\r\n");
            printf("Your answer: ");
        }
        puzzle1.input_active = false;
    }
}

bool is_puzzle1_active(void)
{
    return puzzle1.input_active;
}

// PUZZLE 2: (Placeholder)
void puzzle2_handler(int8_t rssi)
{
    puzzle2_active = true;
    printf("\r\n========== PUZZLE 2 ACTIVATED ==========\r\n");
    printf("RSSI: %d\r\n", rssi);
    // Implement Puzzle 2 logic here
}

// PUZZLE 3: Code Entry Challenge
// Display: Enter a 7-digit code using the keypad
// Task: Input the correct code (default: 1234567)
void puzzle3_handler(int8_t rssi)
{
    puzzle3.solved = false;
    puzzle3.input_active = true;
    puzzle3.input_count = 0;
    
    // Default code: 1234567 (can be randomized or hardcoded)
    strcpy(puzzle3.code, "1234567");
    memset(puzzle3.user_input, 0, PUZZLE_CODE_LENGTH + 1);
    
    printf("\r\n========== PUZZLE 3: CODE ENTRY ==========\r\n");
    printf("Enter the 7-digit code using the keypad\r\n");
    printf("Code: _ _ _ _ _ _ _\r\n");
    printf("\r\nEnter digit: ");
}

void puzzle3_process_keypad(char key)
{
    if (!puzzle3.input_active)
    {
        return;
    }

    if (key >= '0' && key <= '9')
    {
        if (puzzle3.input_count < PUZZLE_CODE_LENGTH)
        {
            puzzle3.user_input[puzzle3.input_count] = key;
            puzzle3.input_count++;
            printf("%c", key);
            
            if (puzzle3.input_count < PUZZLE_CODE_LENGTH)
            {
                printf(" ");
            }
            else
            {
                printf("\r\n");
                
                // Check if code is correct
                if (strcmp(puzzle3.user_input, puzzle3.code) == 0)
                {
                    printf("\r\n========== CORRECT! ==========\r\n");
                    printf("Code %s is correct!\r\n", puzzle3.code);
                    printf("PUZZLE 3 SOLVED!\r\n");
                    puzzle3.solved = true;
                }
                else
                {
                    printf("\r\nWrong code. Try again.\r\n");
                    printf("Enter the 7-digit code using the keypad\r\n");
                    printf("Code: _ _ _ _ _ _ _\r\n");
                    printf("\r\nEnter digit: ");
                }
                
                puzzle3.input_active = false;
            }
        }
    }
}

bool is_puzzle3_active(void)
{
    return puzzle3.input_active;
}

// PUZZLE 4: (Placeholder)
void puzzle4_handler(int8_t rssi)
{
    puzzle4_active = true;
    printf("\r\n========== PUZZLE 4 ACTIVATED ==========\r\n");
    printf("RSSI: %d\r\n", rssi);
    // Implement Puzzle 4 logic here
}

// PUZZLE 5: (Placeholder)
void puzzle5_handler(int8_t rssi)
{
    puzzle5_active = true;
    printf("\r\n========== PUZZLE 5 ACTIVATED ==========\r\n");
    printf("RSSI: %d\r\n", rssi);
    // Implement Puzzle 5 logic here
}
