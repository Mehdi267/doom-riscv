// Exemple d'app minimaliste

#include "../ulib/syscall.h"

int main(void *arg)
{
  printf("Shuting down system");
	(void) arg;
	int a = (int)((long) arg);
  power_off(a);
	return 0;
}