/*
Gestion des tables de pages, PTEs, directory
*/

#ifndef _PAGES_H_
#define _PAGES_H_
#include <stdbool.h>
#include "frame_dist.h"

/*def d'une pte
cf p85 of privileged doc
struct sur 64 bits 
*/


/**
 * @brief page_size_t is used to differentiate different page sizes
 * @param GIGA  refers to the gigabyte page
 * @param MEGA refers to the 2 megabyte page
 * @param KILO refers to the 4096 byte page
*/
typedef enum _page_size {  
                        GIGA,
                        MEGA,
                        KILO  
} page_size_t;


/**
 * @brief page_table_entry is 64 bits value that is associated to every page.
 * With this variable we can identify if we can read or write or exeecute the data that 
 * is in this page. Some other control elements are also present read: 
 * 4.3.1 Addressing and Memory Protection of the priviliged doc
 * @param ppn0 and pp1 and ppn2 are used to associated to every page the physical adresses that it belongs to 
*/
typedef struct __attribute__ ((packed)) page_table_entry {
    unsigned int valid : 1;
    unsigned int read : 1;
    unsigned int write : 1;
    unsigned int exec : 1;
    unsigned int resU: 1;
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
 * @brief A page table is associated to every process and kernel the physical adress of this struct 
 * will be placed in the satp register so that every running element(process or kernel) can locate its 
 * pages 
 * @param pte_list containes the pages that will be associated to this process
 * which are in this case limited to 512 that set by the constant PT_SIZE
 */
typedef struct page_table{
    page_table_entry pte_list[PT_SIZE];
} page_table;

/**
 * @brief page_table_wrapper_t is a wrapper for the page table struct that holds additional inforamtion
 * regarding the page table
 * @param number_of_entries hold the number pte that have been placed in this page table
 */
typedef struct page_table_wrapper{
    page_table* table;
    int number_of_entries;
} page_table_wrapper_t;


/**
 * @brief Create a page table object
 * 
 * @return page_table* an adress to a struct that contains a page table
 * of NULL if any errors occured
 */
page_table *create_page_table();

/**
 * Makes the bit v in the page_table_entry pte valid(true -- 1)
*/
void set_valid(page_table_entry *pte);

/**
 * Makes the bit v in the page_table_entry pte invalid(false -- 0)
*/
void set_invalid(page_table_entry *pte);

/**
 * Checks if a page is valid
*/
bool check_validity(page_table_entry *pte);

/**
 * Sets the approriate bit to the value specified in the bool value
*/
void set_write(page_table_entry *pte, bool write);
void set_read(page_table_entry *pte, bool read);
void set_exec(page_table_entry *pte, bool exec);
void set_user_mode(page_table_entry *pte, bool user_mode);

/**
 * Checks if the pte is a leaf meaning that the R/W/X are not null
*/
bool is_leaf(page_table_entry *pte);

//These three functions set the ppn values in the pte
//  to the given function arguments 
void set_ppn2(page_table_entry *pte, unsigned int ppn);
void set_ppn1(page_table_entry *pte, unsigned int ppn);
void set_ppn0(page_table_entry *pte, unsigned int ppn);

void link_pte(page_table_entry *pte, long unsigned int address);


/**
 * @brief Configures the page entry given as argument
 * using the parameters given also as arguments  
 * @param pte pointer to the page table entry
 * @param address the physical address that will be linked to the page
 * @param read make the page code readable
 * @param write make the page code writable 
 * @param exec make the page code excutable  
 * @param page_size refers to different page size 
 * that we are working with could be GIGA/MEGA/KILO
 */
extern void configure_page_entry(page_table_entry *pte, long unsigned int address, bool read, bool write, bool exec, bool user_mode, page_size_t page_size);

#endif
