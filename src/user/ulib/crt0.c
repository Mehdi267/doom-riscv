/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
 *
 * Bootstrap function linked with every application and used as entry point
 * in the app.
 */

#include "syscall.h"

// extern int main(void *);
extern int main(void *, char*);

/* Tell the compiler the function is in a special section */
void _start(void *, char*) __attribute__((section(".text.init")));

void _start(void * argc, char* argv)
{
	/*
	* Call the "developper" entry point of the application and just
	* ignore the return value. This is the right place to handle this
	* value and call standard library functions if needed.
	*/
	exit(main(argc, argv));
}


