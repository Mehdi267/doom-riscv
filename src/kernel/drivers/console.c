// See LICENSE for license details.

#include "drivers/console.h"
#include "stdlib.h"
#include "stdio.h"

static int default_putchar(int ch)
{
	__asm__ __volatile__("ebreak");
	return 0;
}

static int default_getchar()
{
	__asm__ __volatile__("ebreak");
	return 0;
}

bool is_buffer_full() {
  if (console_dev->last_written_char_index == -1) return 0;
  return (console_dev->last_written_char_index + 1) % BUFFER_SIZE == console_dev->start_of_buffer_index;
}

bool is_buffer_empty() {
  return console_dev->last_written_char_index == -1;
}

void kaddtobuffer(char c){
	if(!is_buffer_full()){ //if buffer is full, ignore
		console_dev->last_written_char_index = (console_dev->last_written_char_index + 1) % BUFFER_SIZE;
		console_dev->buffer[console_dev->last_written_char_index] = c;
	}
}

static int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

size_t buffer_current_size() {
  return mod((console_dev->last_written_char_index -
              console_dev->start_of_buffer_index) +
                 1,
             BUFFER_SIZE);
}

console_device_t console_none = {
		.init = NULL,
		.getchar = default_getchar,
		.putchar = default_putchar
};

console_device_t *console_dev = &console_none;

void register_console(console_device_t *dev)
{
	console_dev = dev;
	if(dev->init)
	{
		dev->init();
		dev->ignore = false; // not reading at first
		dev->last_written_char_index = -1;
		dev->start_of_buffer_index = 0;
		dev->echo = true; // echo is off by default
	  PRINT_GREEN("[INPUT]Uart was configured\n");
  }
}
