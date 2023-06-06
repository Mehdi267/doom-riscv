#include "../drivers/console.h"
#include "../process/helperfunc.h"
#include "../process/scheduler.h"
#include "encoding.h"
#include <stdbool.h>
#include <assert.h>
#include <queue.h>
#include <stdio.h>
#include <string.h>

/**
 * Enable or disable echo on the console.
 *
 * @param on 0 to disable echo, any other value to enable echo.
 */
void cons_echo(int on) {
  // Enable or disable echo on the console based on the 'on' parameter.
  if (!on)
    console_dev->echo = false;
  else
    console_dev->echo = true;
}

/**
 * Print the content of the console buffer.
 */
void print_buffer() {
  printf("buffer: [");
  for (size_t i = 0; i < BUFFER_SIZE; i++)
    printf("%c,", console_dev->buffer[i]);
  printf("]\n");
}

/**
 * Check if an end-of-line character is detected in the console buffer.
 * If length is zero, this function returns 0.
 * Otherwise, it waits for the user to type a complete line terminated by the
 * character 13 (carriage return). It then transfers either the entire line
 * (excluding the character 13), if its length is strictly less than length,
 * or the first 'length' characters of the line, into the 'string' array.
 * Finally, the function returns the number of characters actually transferred.
 * The characters typed but not retrieved remain in the keyboard buffer and will
 * be retrieved in subsequent calls. The end-of-line character (13) is never
 * transmitted to the caller. When 'length' is exactly equal to the number of
 * typed characters (excluding the end-of-line character), the end-of-line marker
 * remains in the buffer. The next call will retrieve an empty line.
 * @return 1 if end-of-line character is detected, 0 otherwise.
 */
unsigned detected_eol() {
  unsigned long temp = console_dev->start_of_buffer_index;
  if (is_buffer_empty()) {
    return 0;
  }
  while (temp != console_dev->last_written_char_index &&
         console_dev->buffer[temp] != '\n') {
    temp++;
    temp %= BUFFER_SIZE;
  }
  if (console_dev->buffer[temp] == '\n')
    return 1;

  return 0;
}

/**
 * Copy characters from the console buffer to a string.
 *
 * @param string  The destination string to copy the characters into.
 * @param length  The maximum number of characters to copy.
 * @return The number of characters actually copied.
 */
unsigned copy(char *dest, unsigned long length) {
  unsigned long index = 0;
  unsigned long temp = console_dev->start_of_buffer_index;
  assert(console_dev->last_written_char_index != -1);
  while (index < length && console_dev->buffer[temp] != '\n' &&
         temp != console_dev->last_written_char_index) {
    dest[index++] = console_dev->buffer[temp];
    console_dev->buffer[temp] = 0;
    temp = (temp + 1) % BUFFER_SIZE;
  }
  if (index >= length) { // the length of the buffer was the problem
    console_dev->start_of_buffer_index = temp;
  } else if (temp == console_dev->last_written_char_index &&
             console_dev->buffer[temp] != '\n') {
    // we read the whole buffer but the '\n' was not the last character
    // so it means that we can read one more character and obviously we still
    // have space in 'dest'
    dest[index++] = console_dev->buffer[temp];
    console_dev->buffer[temp] = 0;
    // reset buffer
    console_dev->start_of_buffer_index = 0;
    console_dev->last_written_char_index = -1;
  } else if (temp == console_dev->last_written_char_index &&
             console_dev->buffer[temp] == '\n') {
    // we read the whole buffer and '\n' was the last character so we reset
    // the buffer
    console_dev->start_of_buffer_index = 0;
    console_dev->last_written_char_index = -1;
  } else if (console_dev->buffer[temp] == '\n') {
    // the loop stopped because we read a '\n' and we didn't reach the top of
    // the buffer, so we increment 'start_of_buffer_index' to ignore '\n' for
    // future 'cons_read' calls
    console_dev->buffer[temp] = 0;
    console_dev->start_of_buffer_index = (temp + 1) % BUFFER_SIZE;
  }
  return index;
}

/**
 * Read characters from the console buffer into a string.
 *
 * This function waits for the user to enter a complete line terminated by the
 * carriage return (ASCII code 13). It then transfers either the entire line
 * (excluding the carriage return) if its length is strictly less than 'length',
 * or the first 'length' characters of the line into the 'string' array.
 *
 * @param string  The destination string to copy the characters into.
 * @param length  The maximum number of characters to read.
 * @return The number of characters actually read and copied.
 */
unsigned long cons_read(char *string, unsigned long length) {
  if (!length)
    return 0;
  while (!detected_eol() && !is_buffer_full() && length > buffer_current_size()) {
    process *proc = get_current_process();
    proc->state = BLOCKEDIO;
    queue_add(proc, &blocked_io_process_queue, process, next_prev, prio);
    scheduler();
  }
  return copy(string, length);
}

