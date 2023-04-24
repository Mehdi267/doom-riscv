/*
 * Projet PCSEA RISC-V
 *
 * Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019 - 2020
 *
 * See license for license details.
 */

#include "stdint.h"
#include "drivers/auxval.h"
#include "drivers/console.h"

enum {
	/* UART Registers */
	UART_REG_TXFIFO = 0,
	UART_REG_RXFIFO = 1,
	UART_REG_TXCTRL = 2,
	UART_REG_RXCTRL = 3,
	UART_REG_IE = 4,
	UART_REG_IP = 5,
	UART_REG_DIV = 6,

	/* TXCTRL register */
	UART_TXEN = 1,
	UART_TXSTOP = 2,

	/* RXCTRL register */
	UART_RXEN = 1,

	/* IP register */
	UART_IP_TXWM = 1,
	UART_IP_RXWM = 2
};

static volatile int *uart;

static void cep_uart_init()
{
	uart = (int *) (void *) getauxval(CEP_UART0_CTRL_ADDR);
	uint32_t uart_freq = getauxval(UART0_CLOCK_FREQ);
	uint32_t baud_rate = getauxval(UART0_BAUD_RATE);
	uint32_t divisor = uart_freq / baud_rate - 1;
	uart[UART_REG_DIV] = divisor;
	uart[UART_REG_TXCTRL] = UART_TXEN;
	uart[UART_REG_RXCTRL] = UART_RXEN;
	uart[UART_REG_IE] = 2;
	//making sure buffer is empty
	console_dev->top_ptr=0;
}

static int cep_uart_getchar()
{
	int ch = uart[UART_REG_RXFIFO];
	if(ch < 0) return -1;
	return ch;
}

static int cep_uart_putchar(int ch)
{
	while (uart[UART_REG_TXFIFO] < 0);
	return uart[UART_REG_TXFIFO] = ch & 0xff;
}

console_device_t console_cep_uart = {
		cep_uart_init,
		cep_uart_getchar,
		cep_uart_putchar
};
