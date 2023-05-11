// See LICENSE for license details.
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <stdbool.h>
#include <stdio.h>

#pragma once

#define BUFFER_SIZE 20
#define EOL 13

typedef struct console_device {
	void (*init)();
	int (*getchar)();
	int (*putchar)(int);
	//void (*add_to_buffer)(char);
	bool ignore;
	char buffer[BUFFER_SIZE]; //temp
	int last_written_char_index;
	int start_of_buffer_index;
	bool echo;
} console_device_t;

/*
 * console_dev global variable
 * This variable is useful to call console functions.
 */
extern console_device_t *console_dev;

static inline int kgetchar()
{
	return console_dev->getchar();
}

static inline int kputchar(int ch)
{
	/* if(console_dev->echo) */
	return console_dev->putchar(ch);
	/* return 0; */
}

/**
 *@brief add char c to the buffer if buffer is not full
*/
void kaddtobuffer(char c);
/**
 *@brief returns whether buffer is full or not
*/
bool is_buffer_full();
/**
 *@brief returns whether buffer is empty or not
*/
bool is_buffer_empty();

size_t buffer_current_size();
/*
 * Console drivers
 */
extern console_device_t console_none;
extern console_device_t console_htif;
extern console_device_t console_ns16550a;
extern console_device_t console_sifive_uart;
extern console_device_t console_cep_uart;
extern console_device_t console_semihost;

void register_console(console_device_t *dev);


#endif
