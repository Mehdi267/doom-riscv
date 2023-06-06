#include "keyboard.h"
#include "../drivers/console.h"
#include "../process/process.h"
#include "../process/scheduler.h"
#include "queue.h"
#include <assert.h>
#include <stdio.h>

/**
 * Check if a character is printable.
 *
 * @param c The character to check.
 * @return true if the character is printable, false otherwise.
 */
bool is_printable(char c) {
  return ((int)c < 127) && ((int)c > 31);
}

/**
 * Delete the last character from the console buffer and update the display.
 *
 * TODO: Handle the case of TAB.
 */
void delete_last() {
  console_dev->buffer[console_dev->last_written_char_index] = 0;
  console_dev->last_written_char_index--;
  console_dev->putchar(BSH);
  console_dev->putchar(' ');
  console_dev->putchar(BSH);
}

/**
 * Handle the keyboard interrupt.
 *
 * In the keyboard driver, whenever a key is pressed and a character sequence is
 * entered into the buffer, it should be displayed on the screen unless echo mode
 * has been disabled with 'cons_echo'.
 */
void handle_keyboard_interrupt() {
  char c = kgetchar();
  if (is_printable(c)) {
    kaddtobuffer(c);
    if (console_dev->echo)
      console_dev->putchar(c); // echo
  } else if ((int)c == HT) {
    // Put space characters to erase the previous input.
    int nb_spaces = 8 - (console_dev->last_written_char_index % 8);
    for (int i = 0; i < nb_spaces; i++) {
      console_dev->putchar(' ');
      kaddtobuffer(' ');
    }
  } else if (c == BS || c == BSH) {
    if (console_dev->last_written_char_index != -1 &&
        console_dev->buffer[console_dev->last_written_char_index - 1] != '\r') {
      delete_last();
    }
  } else if (c == LF) {
    kaddtobuffer(c);
    if (console_dev->echo)
      console_dev->putchar('\n'); // putchar(R) only goes to the beginning of the line
  } else if (c == DL) {
    // Deletes the current line.
    while (console_dev->last_written_char_index != -1 &&
           console_dev->buffer[console_dev->last_written_char_index - 1] != '\r') {
      delete_last();
    }
  } else if (c == DW) {
    // Deletes the current word.
    while (console_dev->last_written_char_index != -1 &&
           console_dev->buffer[console_dev->last_written_char_index - 1] == ' ') {
      if (console_dev->echo)
        delete_last();
    }
    while (console_dev->last_written_char_index != -1 &&
           console_dev->buffer[console_dev->last_written_char_index - 1] != ' ') {
      if (console_dev->echo)
        delete_last();
    }
  } else if (c == CR) {
    kaddtobuffer('\n');
    if (console_dev->echo)
      console_dev->putchar('\n'); // echo
  }
  process *next = queue_out(&blocked_io_process_queue, process, next_prev);
  if (next) {
    next->state = ACTIVATABLE;
    queue_add(next, &activatable_process_queue, process, next_prev, prio);
    scheduler();
  }
}
