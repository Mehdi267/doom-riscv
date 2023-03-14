#include "pages.h"
#include "frame_dist.h"
#include "../process/helperfunc.h"
#include "process/process.h"
#include <stdbool.h>
#include <assert.h>
#include <string.h>

/**
 * @brief Create a page table struct thus allocating 4096 bytes in which we can place
 * 512 pages entries
 * @note In the satp register we will place value retuned from here
 * @return page_table* 
 */
page_table *create_page_table(){
    page_table *ptr = (page_table *) get_frame(); //Allocates data for the page table
    if (ptr == NULL){
        return NULL;
    }
    //We clear the page frame so that we can be sure that random noise
    //will not interfer with addresses that we will work with
    memset(ptr, 0, FRAME_SIZE);
    return ptr;
}

void configure_page_entry(page_table_entry *pte, long unsigned int address, 
                            bool read,
                            bool write,
                            bool exec,
                            bool user_mode,
                            page_type_t page_type){
    //address is relative to satp.ppn
    // assert(read || !write); //or the page would be invalid
    // assert(read || exec); //or the page is not a leaf
    set_valid(pte);
    set_read(pte, read);
    set_write(pte, write);
    set_exec(pte, exec);
    set_user_mode(pte, user_mode);
    //in accordance to point 6 of 4.3.2, we have to set ppn[1] and ppn[0] to 0
    //The value of these ppn are not relevant since we only use pp2 
    //to get the adress of the giga byte page
    switch (page_type){
        case GIGA:
            set_ppn0(pte, 0);
            set_ppn1(pte, 0);
            set_ppn2(pte, address);
            break;
        //These value are not right now but logically we nned to write to the 
        //pp1 and pp2 when working with mega pages and all the locations 
        //when working with kb pages
        case MEGA:
            set_ppn0(pte, 0);
            //We must take only certain bytes here
            set_ppn1(pte, address);
            set_ppn2(pte, address);
            break;
        case KILO:
            set_ppn0(pte, address);
            set_ppn1(pte, address);
            set_ppn2(pte, address);
            break;
        default:
            break;
    }
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

void set_user_mode(page_table_entry *pte, bool user_mode){
    pte->resU = user_mode ? 1 : 0;
}

void set_invalid(page_table_entry *pte){
    pte->valid = 0;
}



//link pte to address
void link_pte(page_table_entry *pte, void *address){
    address = (void *)((unsigned long int)address >> FRAME_SIZE_EXP); //we do not write the 12 zeros of the alignment
    pte->ppn0 = MASK_ADDRESS(address, PPN0_MASK);
    address = (void *)((unsigned long int)address >> PPN0_SIZE);
    pte->ppn1 = MASK_ADDRESS(address, PPN1_MASK);
    address = (void *)((unsigned long int)address >> PPN1_SIZE);
    pte->ppn2 = MASK_ADDRESS(address, PPN2_MASK);
}