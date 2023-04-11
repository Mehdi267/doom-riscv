#pragma once

#include "stdint.h"
#include "stdbool.h"

#define CLINT_MTIME 0xbff8
#define CLINT_MTIMECMP 0x4000
#define CLINT_STIME 0xc000
#define CLINT_STIMECMP 0xc008
#define CLINT_UTIME 0xd000
#define CLINT_UTIMECMP 0xd008

typedef struct clint_device {
	uint64_t clk_freq;
	uint64_t base_addr;
	bool supervisor_clint_available;
	bool user_clint_available;
} clint_device_t;

/*
 * clint_dev global variable
 * This variable is useful for fetching base addr and clock freq informations on the CLINT of the machine.
 */
extern clint_device_t *clint_dev;

/*
 * CLINT drivers
 */
extern clint_device_t clint_none;
extern clint_device_t spike_clint;
extern clint_device_t sifive_clint;
extern clint_device_t cep_clint;
extern clint_device_t virt_clint;

void register_clint(clint_device_t *dev);

/*
 * CLINT macros
 */

#define get_mtime() (*(uint64_t*)((uint64_t)clint_dev->base_addr + CLINT_MTIME))

#define get_mtimecmp() (*(uint64_t*)((uint64_t)clint_dev->base_addr + CLINT_MTIMECMP))

#define set_mtimecmp(value)                                                    \
({                                                                            \
    *(uint64_t*)(clint_dev->base_addr + CLINT_MTIMECMP) = (uint64_t)(value);            \
})

#define get_stime() (*(uint64_t*)((uint64_t)clint_dev->base_addr + CLINT_STIME))

#define get_stimecmp() (*(uint64_t*)((uint64_t)clint_dev->base_addr + CLINT_STIMECMP))

#define set_stimecmp(value)                                                    \
({                                                                            \
    *(uint64_t*)(clint_dev->base_addr + CLINT_STIMECMP) = (uint64_t)(value);            \
})

#define get_utime() (*(uint64_t*)((uint64_t)clint_dev->base_addr + CLINT_UTIME))

#define get_utimecmp() (*(uint64_t*)((uint64_t)clint_dev->base_addr + CLINT_UTIMECMP))

#define set_utimecmp(value)                                                    \
({                                                                            \
    *(uint64_t*)(clint_dev->base_addr + CLINT_UTIMECMP) = (uint64_t)(value);            \
})

