/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
 *
 * Defines structure to find user space programs.
 */

#include "string.h"
#include "stdio.h"
#include "assert.h"

#include "userspace_apps.h"

const struct uapps *find_app(const char *name)
{
	//Iterator
	int app = 0;
	while (symbols_table[app].name != NULL){
    if (strcmp(symbols_table[app].name, name) == 0){
      //We found the app
      return &symbols_table[app];
    }
		app++;
	}
	return NULL;
}

