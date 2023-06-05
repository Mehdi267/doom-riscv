#include "pages.h"
#include "frame_dist.h"
#include "../process/helperfunc.h"
#include "process/process.h"
#include <stdbool.h>
#include <assert.h>
#include <string.h>

// Create a page table struct and allocate 4096 bytes to hold 512 page entries
// The value returned from this function will be placed in the satp register
// Returns a pointer to the page table or NULL if any errors occurred
page_table *create_page_table() {
    page_table *ptr = (page_table *)get_frame(); // Allocates memory for the page table
    if (ptr == NULL) {
        return NULL;
    }
    // Clear the page frame to ensure no interference with addresses
    memset(ptr, 0, FRAME_SIZE);
    return ptr;
}

void configure_page_entry(page_table_entry *pte, long unsigned int address,
                          bool read, bool write, bool exec, bool user_mode,
                          page_size_t page_size) {
    // Address is relative to satp.ppn
    set_valid(pte);
    set_read(pte, read);
    set_write(pte, write);
    set_exec(pte, exec);
    set_user_mode(pte, user_mode);
    // In accordance with point 6 of 4.3.2, set ppn[1] and ppn[0] to 0
    // The value of these ppn is not relevant since we only use pp2 to get the address of the gigabyte page
    switch (page_size) {
        case GIGA:
            link_pte(pte, address);
            set_ppn0(pte, 0);
            set_ppn1(pte, 0);
            break;
        case MEGA:
            link_pte(pte, address);
            set_ppn0(pte, 0);
            break;
        case KILO:
            link_pte(pte, address);
            break;
        default:
            break;
    }
}

// Set the valid bit and clear reserved bits in the page table entry
void set_valid(page_table_entry *pte) {
    pte->valid = 1;
    // Clear reserved bits (cf 4.3.2.3)
    pte->resA = 0;
    pte->resU = 0;
    pte->resD = 0;
    pte->reserved = 0;
}

// Check if a page table entry is valid
// Returns true if the entry is valid and reserved bits are all zero, false otherwise
bool check_validity(page_table_entry *pte) {
    return pte->valid && !(pte->resA || pte->resD || pte->resU || pte->reserved);
}

// Check if a page table entry is a leaf (R/W/X are not null)
// Returns true if the entry is a leaf, false otherwise
bool is_leaf(page_table_entry *pte) {
    return pte->read || pte->exec;
}

// Set the write flag in the page table entry
void set_write(page_table_entry *pte, bool write) {
    pte->write = write ? 1 : 0;
}

// Set the ppn0 value in the page table entry
void set_ppn0(page_table_entry *pte, unsigned int ppn) {
    assert(ppn < (unsigned int)0x1FF); // Making sure the ppn holds on 9 bits
    pte->ppn0 = ppn;
}

// Set the ppn1 value in the page table entry
void set_ppn1(page_table_entry *pte, unsigned int ppn) {
    assert(ppn < (unsigned int)0x1FF); // Making sure the ppn holds on 9 bits
    pte->ppn1 = ppn;
}

// Set the ppn2 value in the page table entry
void set_ppn2(page_table_entry *pte, unsigned int ppn) {
    assert(ppn < (unsigned int)0x3FFFFFF); // Making sure the ppn holds on 26 bits
    pte->ppn2 = ppn;
}

// Set the exec flag in the page table entry
void set_exec(page_table_entry *pte, bool exec) {
    pte->exec = exec ? 1 : 0;
}

// Set the read flag in the page table entry
void set_read(page_table_entry *pte, bool read) {
    pte->read = read ? 1 : 0;
}

// Set the user mode flag in the page table entry
void set_user_mode(page_table_entry *pte, bool user_mode) {
    pte->resU = user_mode ? 1 : 0;
}

// Set the valid bit to 0 in the page table entry, marking it as invalid
void set_invalid(page_table_entry *pte) {
    pte->valid = 0;
}

/*
 * Links the given page table entry (pte) to the specified address
 * The address is right-shifted by FRAME_SIZE_EXP to ignore the 12 zeros of alignment
 * The ppn0, ppn1, and ppn2 values of the pte are updated based on the address
 */
void link_pte(page_table_entry *pte, long unsigned int address) {
    address = address >> FRAME_SIZE_EXP;
    pte->ppn0 = MASK_ADDRESS(address, PPN0_MASK);
    address = address >> PPN0_SIZE;
    pte->ppn1 = MASK_ADDRESS(address, PPN1_MASK);
    address = address >> PPN1_SIZE;
    pte->ppn2 = MASK_ADDRESS(address, PPN2_MASK);
}
