#include "pages.h"
#include "frame_dist.h"
#include <stdbool.h>
#include <assert.h>

page_table *create_page_table(){
    page_table *ptr = (page_table *) get_frame(); //alloue 4096 octets pour la page
    return ptr;
}

page_table *init_directory(){
  page_table *directory_ptr = create_page_table();
  page_table_entry *pte = directory_ptr->pte_list+0; // La 1ère PTE du directory pointera vers la gigapage
  set_gigapage(pte, 0, true, true, false); //gigapage is RW, with on offset of 0
  return directory_ptr;
}

void set_gigapage(page_table_entry *pte, unsigned int adress, bool read, bool write, bool exec){
    //adress is relative to satp.ppn
    assert(read || !write); //or the page would be invalid
    assert(read || exec); //or the page is not a leaf
    set_valid(pte);
    set_read(pte, read);
    set_write(pte, write);
    set_exec(pte, exec);
    //in accordance to point 6 of 4.3.2, we have to set ppn[1] and ppn[0] to 0
    set_ppn0(pte, 0);
    set_ppn1(pte, 0);
    set_ppn2(pte, adress);
}


//to specify a pte is valid
void set_valid(page_table_entry *pte){
    pte->valid = 1;
    //il faut également mettre les bits réservés à 0 (cf 4.3.2.3)
    pte->resA = 0;
    pte->resU = 0;
    pte->resD = 0;
    pte->reserved = 0;
}


bool check_validity(page_table_entry *pte){
    return pte->valid && !(pte->resA || pte->resD || pte->resU || pte->reserved);
}

bool is_leaf(page_table_entry *pte){
    return pte->read || pte->exec;
}

//setters for different fields of the ptes
void set_write(page_table_entry *pte, bool write){
    pte->write = write ? 1 : 0;
}

void set_ppn0(page_table_entry *pte, unsigned int ppn){
    assert(ppn < (unsigned int)0x1FF); //making sure the ppn holds on 9 bits
    pte->ppn0 = ppn;
}

void set_ppn1(page_table_entry *pte, unsigned int ppn){
    assert(ppn < (unsigned int)0x1FF); //making sure the ppn holds on 9 bits
    pte->ppn1 = ppn;
}

void set_ppn2(page_table_entry *pte, unsigned int ppn){
    assert(ppn < (unsigned int)0x3FFFFFF); //making sure the ppn holds on 26 bits
    pte->ppn2 = ppn;
}

void set_exec(page_table_entry *pte, bool exec){
    pte->exec = exec ? 1 : 0;
}

void set_read(page_table_entry *pte, bool read){
    pte->read = read ? 1 : 0;
}

void set_invalid(page_table_entry *pte){
    pte->valid = 0;
}