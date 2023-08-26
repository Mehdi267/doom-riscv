#include <stdio.h>

void printb(const void* data, size_t size) {
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size; ++i) {
        for (int j = 0; j <= 7; ++j) {
            unsigned char bit = (bytes[i] >> j) & 1;
            printf("%u", bit);
        }
    }
    printf("\n");
}

void print_block(void* data, size_t size){
  printf("\n");
  printf("##########Block#############\n");
  unsigned int* ptr_64 = (unsigned int *) data;
  for (int i = 0; i < 512/sizeof(unsigned int); i++) {
    printf("%x ", ptr_64[i]);
    if (((i) % 16 && (i)>0) == 0) {
        printf("\n");
    }
  }
  printf("\n");
}

