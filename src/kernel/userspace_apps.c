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
#include "fs/fs_api.h"

//Some files that should not be copied to disk
//these files are used to send data from current 
const char *black_list_copy[] = {
  "copy_files",
};


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

int write_user_apps_fs(){
  int app = 0;
  while (symbols_table[app].name != NULL){
    int code_size = (int) ((long) symbols_table[app].end - 
        (long) symbols_table[app].start);
    char dir[] = "/bin/";
    for (int file = 0;
          file < sizeof(black_list_copy)/sizeof(char *);
          file++){
      if (strcmp(black_list_copy[file], symbols_table[app].name) == 0){
        continue;
      }
    }
    uint64_t name_len = strlen(dir)+strlen(symbols_table[app].name)+1;
    char buf[name_len];
    memcpy(buf, dir, strlen(dir));
    memcpy(buf+strlen(dir),
          symbols_table[app].name, 
          strlen(symbols_table[app].name));
    buf[name_len-1] = 0;
    int file_fd = open(0, buf, O_CREAT | O_TRUNC | O_RDWR, 0);
    if (file_fd<0){return -1;}
    if (write(file_fd, symbols_table[app].start, code_size)<code_size){
      return -1;
    }
    close(file_fd);
		app++;
	}
  return 0;
}