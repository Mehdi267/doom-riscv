// See LICENSE for license details.

#pragma once

#define EOL 13

typedef struct console_device {
	void (*init)();
	int (*getchar)();
	int (*putchar)(int);
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
	return console_dev->putchar(ch);
}

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


