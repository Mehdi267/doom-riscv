// Exemple d'app minimaliste

#include "syscall.h"

int main(void *arg)
{
  printf("Shuting down system\n");
	(void) arg;
	int a = (int)((long) arg);
  power_off(a);
	return 0;
}