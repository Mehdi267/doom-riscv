#include "stdio.h"
#include "virtual_memory.h"

extern char _text_start[];
extern char _text_end[];
extern char _rodata_start[];
extern char _rodata_end[];
extern char _data_start[];
extern char _data_end[];
extern char _bss_start[];
extern char _bss_end[];
extern char _heap_start[];
extern char _heap_end[];
extern char _free_memory_start[];
extern char _memory_start[];
extern char _memory_end[];

int print_mem_symbols() {
    printf("Symbol values:\n");
    printf("_text_start: %p\n", _text_start);
    printf("_text_end: %p\n", _text_end);
    printf("_rodata_start: %p\n", _rodata_start);
    printf("_rodata_end: %p\n", _rodata_end);
    printf("_data_start: %p\n", _data_start);
    printf("_data_end: %p\n", _data_end);
    printf("_bss_start: %p\n", _bss_start);
    printf("_bss_end: %p\n", _bss_end);
    printf("_heap_start: %p\n", _heap_start);
    printf("_heap_end: %p\n", _heap_end);
    printf("_free_memory_start: %p\n", _free_memory_start);
    printf("_memory_start: %p\n", _memory_start);
    printf("_memory_end: %p\n", _memory_end);
    return 0;
}