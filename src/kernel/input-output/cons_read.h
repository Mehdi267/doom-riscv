#ifndef _CONSREAD_H_
#define _CONSREAD_H_

unsigned long copy(char *dest, unsigned long length, bool copy_s);
unsigned long cons_read(const char *string, unsigned long length);
void cons_echo(int on);

#endif