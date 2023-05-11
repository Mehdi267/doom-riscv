// #include "../tests/sys_api.h"
#include "../drivers/console.h"
#include "../process/helperfunc.h"
#include "../process/scheduler.h"
#include "encoding.h"
#include "stdbool.h"
#include <assert.h>
#include <queue.h>
#include <stdio.h>
#include <string.h>

void cons_echo(int on) {
  // active (on!=0) ou desactive (on=0) l'echo sur la console
  if (!on)
    console_dev->echo = false;
  else
    console_dev->echo = true;
}

void print_buffer() {
  printf("buffer: [");
  for (size_t i = 0; i < BUFFER_SIZE; i++)
    printf("%c,", console_dev->buffer[i]);
  printf("]\n");
}

/**
Si length est nul, cette fonction retourne 0.
Sinon, elle attend que l'utilisateur ait tapé une ligne complète terminée par le
caractère 13 puis transfère dans le tableau string soit la ligne entière
(caractère 13 non compris), si sa longueur est strictement inférieure à length,
soit les length premiers caractères de la ligne. Finalement, la fonction
retourne à l'appelant le nombre de caractères effectivement transmis. Les
caractères frappés et non prélevés restent dans le tampon associé au clavier et
seront prélevés aux appels suivants. Le caractère de fin de ligne (13) n'est
jamais transmis à l'appelant. Lorsque length est exactement égal au nombre de
caractères frappés, fin de ligne non comprise, le marqueur de fin de ligne reste
dans le tampon. Le prochain appel récupèrera une ligne vide.
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
unsigned copy(char *string, unsigned long length) {
  unsigned long i = 0;
  unsigned long temp = console_dev->start_of_buffer_index;
  assert(console_dev->last_written_char_index != -1);
  while (i < length && console_dev->buffer[temp] != '\n' &&
         temp != console_dev->last_written_char_index) {
    string[i++] = console_dev->buffer[temp];
    console_dev->buffer[temp] = 0;
    temp = (temp + 1) % BUFFER_SIZE;
  }
  if (i >= length) { // the length of the buffer was the problem
    console_dev->start_of_buffer_index = temp;
  } else if (temp == console_dev->last_written_char_index &&
             console_dev->buffer[temp] != '\n') {
    // we read the whole buffer but the '\n' was not the last character
    // so it means that we can read one more character and obviously we still
    // have spece in string
    string[i++] = console_dev->buffer[temp];
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
    // the loop stopped bc we read a '\n' and we didnt arrived to the top of
    // the buffer so we increment start_of_buffer_index to ignore '\n\ for
    // future cons_read
    console_dev->buffer[temp] = 0;
    console_dev->start_of_buffer_index = (temp + 1) % BUFFER_SIZE;
  }
  return i;
}

unsigned long cons_read(char *string, unsigned long length) {
  if (!length)
    return 0;
  while(!detected_eol() && !is_buffer_full() && length > buffer_current_size()) {
    process *proc = get_current_process();
    proc->state = BLOCKEDIO;
    queue_add(proc, &blocked_io_process_queue, process, next_prev, prio);
    scheduler();
  }
  return copy(string, length);
}
