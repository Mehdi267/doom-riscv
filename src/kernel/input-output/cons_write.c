// See LICENSE for license details.

#include <stdio.h>
#include "drivers/console.h"

int cons_write(const char *str, long size)
{
	int res = 0;
	for (int i = 0; i < size; i++) {
		if ((kputchar(str[i])) >= 0) {
			res++;
		} else
			break;
	}
	return res;
}

