// See LICENSE for license details.

#include "drivers/console.h"
#include "stdlib.h"

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
/*
void add_to_buffer(char c)
{
	if(!console_dev->ignore){
		if(!(console_dev->top_ptr >= 128)){ //if buffer is full, ignore
			console_dev->buffer[console_dev->top_ptr] = c;
			console_dev->top_ptr ++;
		}
	}
}*/

void kaddtobuffer(char c){
	if(!console_dev->ignore){ //if buffer is full, ignore
		console_dev->buffer[console_dev->top_ptr] = c;
		console_dev->top_ptr ++;
		if(console_dev->top_ptr == 20){
			//buffer is full
			cons_echo(0);
			console_dev->ignore = true;
		}
	}
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
		dev->top_ptr = 0;
		dev->echo = true; // echo is off by default
	}
}
