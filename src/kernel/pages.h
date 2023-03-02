/*
Gestion des tables de pages, PTEs, directory
*/

#ifndef _PAGES_H_
#define _PAGES_H_
#include <stdbool.h>

/*def d'une pte
cf p85 of privileged doc
struct sur 64 bits 
*/
typedef struct page_table_entry{
    unsigned int ppn2 : 26; //pas au "bon" endroit, mais c'est normal, sinon la struct prend plus de 64 bits
    unsigned int valid : 1;
    unsigned int read : 1;
    unsigned int write : 1;
    unsigned int exec : 1;
    unsigned int resU: 1;
    unsigned int global : 1;
    //32
    unsigned int resA : 1;
    unsigned int resD : 1;
    unsigned int rsw : 2;
    unsigned int ppn0 : 9;
    unsigned int ppn1 : 9;
    unsigned int reserved : 7;
    unsigned int pbmt : 2;
    unsigned int n : 1;
    //64
} page_table_entry;

//def d'une page table
typedef struct page_table{
    //512PTEs de 8 octets
    page_table_entry pte_list[512];
} page_table;

page_table *create_page_table();

void set_valid(page_table_entry *pte);
void set_invalid(page_table_entry *pte);

bool check_validity(page_table_entry *pte);

void set_write(page_table_entry *pte, bool write);
void set_read(page_table_entry *pte, bool read);
void set_exec(page_table_entry *pte, bool exec);

bool is_leaf(page_table_entry *pte);

void set_ppn2(page_table_entry *pte, unsigned int ppn);
void set_ppn1(page_table_entry *pte, unsigned int ppn);
void set_ppn0(page_table_entry *pte, unsigned int ppn);

void set_gigapage(page_table_entry *pte, long unsigned int adress, bool read, bool write, bool exec);

/**
* Crée le directory utilisé par l'ensemble des process kernels 
* Crée la page contenant la mémoire du noyau et les directories
* Cette page est référencée par une pte du directory
* renvoie l'adresse physique du directory
*/
page_table *init_directory();

#endif