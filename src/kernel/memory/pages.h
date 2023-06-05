/*
 * Gestion des tables de pages, PTEs, directory
 */

#ifndef _PAGES_H_
#define _PAGES_H_

#include <stdbool.h>
#include "frame_dist.h"

/**
 * @brief page_size_t is used to differentiate different page sizes.
 *        - GIGA: refers to the gigabyte page.
 *        - MEGA: refers to the 2 megabyte page.
 *        - KILO: refers to the 4096 byte page.
 */
typedef enum _page_size {
    GIGA,
    MEGA,
    KILO
} page_size_t;

/**
 * @brief page_table_entry is a 64-bit value associated with every page.
 *        It determines the accessibility and control elements of the page.
 *        Refer to section 4.3.1 "Addressing and Memory Protection" of the privileged document.
 *        - valid: indicates if the entry is valid.
 *        - read: allows reading from the page.
 *        - write: allows writing to the page.
 *        - exec: allows executing code in the page.
 *        - resU: reserved for future use.
 *        - global: indicates a global page that is shared among processes.
 *        - resA: reserved for future use.
 *        - resD: reserved for future use.
 *        - rsw: reserved bits.
 *        - ppn0, ppn1, ppn2: physical page number (physical address) associated with the page.
 *        - reserved: reserved bits.
 *        - pbmt: page-based memory type.
 *        - n: indicates a non-leaf node in the page table.
 */
typedef struct __attribute__ ((packed)) page_table_entry {
    unsigned int valid : 1;
    unsigned int read : 1;
    unsigned int write : 1;
    unsigned int exec : 1;
    unsigned int resU : 1;
    unsigned int global : 1;
    unsigned int resA : 1;
    unsigned int resD : 1;
    unsigned int rsw : 2;
    unsigned int ppn0 : 9;
    unsigned int ppn1 : 9;
    unsigned int ppn2 : 26;
    unsigned int reserved : 7;
    unsigned int pbmt : 2;
    unsigned int n : 1;
} page_table_entry;

/**
 * @brief A page table is associated with every process and the kernel.
 *        The physical address of this struct will be placed in the satp register
 *        so that every running element (process or kernel) can locate its pages.
 *        - pte_list: contains the page table entries (limited to PT_SIZE, defined constant).
 */
typedef struct page_table {
    page_table_entry pte_list[PT_SIZE];
} page_table;

/**
 * @brief page_table_wrapper_t is a wrapper for the page table struct that holds additional information
 *        regarding the page table.
 *        - table: pointer to the page table.
 *        - number_of_entries: number of page table entries that have been placed in this page table.
 */
typedef struct page_table_wrapper {
    page_table* table;
    int number_of_entries;
} page_table_wrapper_t;

/**
 * @brief Create a page table object.
 * 
 * @return page_table* An address to a struct that contains a page table,
 *         or NULL if any errors occurred.
 */
page_table *create_page_table();

/**
 * @brief Makes the valid bit in the page_table_entry pte true (1).
 */
void set_valid(page_table_entry *pte);

/**
 * @brief Makes the valid bit in the page_table_entry pte false (0).
 */
void set_invalid(page_table_entry *pte);

/**
 * @brief Checks if a page is valid.
 *
 * @return bool true if the page is valid, false otherwise.
 */
bool check_validity(page_table_entry *pte);

/**
 * @brief Sets the appropriate bit in the page_table_entry pte to the specified bool value.
 */
void set_write(page_table_entry *pte, bool write);
void set_read(page_table_entry *pte, bool read);
void set_exec(page_table_entry *pte, bool exec);
void set_user_mode(page_table_entry *pte, bool user_mode);

/**
 * @brief Checks if the page_table_entry pte is a leaf, meaning that the R/W/X bits are not null.
 *
 * @return bool true if the page table entry is a leaf, false otherwise.
 */
bool is_leaf(page_table_entry *pte);

/**
 * @brief Sets the ppn2 value in the page_table_entry pte to the given ppn.
 */
void set_ppn2(page_table_entry *pte, unsigned int ppn);

/**
 * @brief Sets the ppn1 value in the page_table_entry pte to the given ppn.
 */
void set_ppn1(page_table_entry *pte, unsigned int ppn);

/**
 * @brief Sets the ppn0 value in the page_table_entry pte to the given ppn.
 */
void set_ppn0(page_table_entry *pte, unsigned int ppn);

/**
 * @brief Links the page table entry pte to the given physical address.
 */
void link_pte(page_table_entry *pte, long unsigned int address);

/**
 * @brief Configures the page entry specified by the pte argument using the provided parameters.
 *
 * @param pte The pointer to the page table entry.
 * @param address The physical address linked to the page.
 * @param read Specifies whether the page is readable.
 * @param write Specifies whether the page is writable.
 * @param exec Specifies whether the page is executable.
 * @param user_mode Specifies whether the page is in user mode.
 * @param page_size Refers to the different page size we are working with (GIGA/MEGA/KILO).
 */
extern void configure_page_entry(page_table_entry *pte, long unsigned int address, bool read, bool write, bool exec, bool user_mode, page_size_t page_size);

#endif
