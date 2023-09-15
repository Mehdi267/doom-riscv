#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#define BSH 8
#define BSK 127
#define R 13 // Carriage return
#define DL 21 // Delete line
#define DW 23 // Delete word
#define HT 9 // Tab
#define LF 10 // Next line, column 0
#define CR 13 // Move cursor to the beginning of the current line

/**
 * @brief Handle the keyboard interrupt.
 *
 * This function is called whenever a keyboard interrupt occurs. It processes
 * the input character and performs the necessary actions based on the input.
 * - If the input character is a printable character, it adds it to the buffer
 *   and, if echo mode is enabled, displays the character on the screen.
 * - If the input character is a tab (HT), it adds spaces to the buffer to
 *   simulate tab functionality and echoes the spaces on the screen.
 * - If the input character is a backspace (BS or BSH), it deletes the last
 *   character from the buffer and echoes the appropriate backspace sequence on
 *   the screen.
 * - If the input character is a line feed (LF), it adds it to the buffer and,
 *   if echo mode is enabled, moves the cursor to the beginning of the next line.
 * - If the input character is a delete line (DL), it deletes the entire line
 *   from the buffer and echoes the appropriate backspace sequence on the screen.
 * - If the input character is a delete word (DW), it deletes the current word
 *   from the buffer and echoes the appropriate backspace sequence on the screen.
 * - If the input character is a carriage return (CR), it adds a newline
 *   character to the buffer and, if echo mode is enabled, moves the cursor to
 *   the beginning of the next line.
 *
 * After processing the input, this function checks if there are any processes
 * blocked on input and activates the next process in line.
 */
void handle_keyboard_interrupt();

#endif
