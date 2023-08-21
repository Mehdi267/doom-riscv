
#include "sysapi.h"

void print_memory(const void *start_address, const void *end_address) {
  const char *current_address = (const char *)start_address;
  printf("prining memory diff %ld\n",  
        ((unsigned long)end_address - (unsigned long)start_address));
  unsigned long limit = ((unsigned long)end_address - (unsigned long)start_address);
  unsigned long iter = 0;
  while (iter < limit) {
    printf("%c", *current_address);
    iter++;
    current_address++;
  }
  printf("\n");
  current_address = (const char *)start_address;
  iter = 0;
  while (iter < limit) {
    printf("%.2x ", (unsigned char)*current_address);
    iter++;
    if (iter%4 == 0){
      printf(" ");
    }
    current_address++;
  }
  printf("\n");
}

int main(int argc, char *argv[]) {
  assert(argc == 4);
  const char *expected_argv[] = {"program1", "arg1_modified", 
        "arg2_modified", "arg3_modified", NULL};
  for (int i = 0; i < argc; i++) {
    assert(memcmp(expected_argv[i], argv[i], strlen(expected_argv[i])) == 0);
  }
  return 0;
}