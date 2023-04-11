// See LICENSE for license details.

#pragma once

typedef struct plic_device {
	void (*init)();
} plic_device_t;

/*
 * plic_dev global variable
 * This variable is useful to init PLIC on the machine.
 */
extern plic_device_t *plic_dev;

/*
 * PLIC drivers
 */
extern plic_device_t plic_none;
extern plic_device_t cep_plic;
extern plic_device_t sifive_plic;
extern plic_device_t virt_plic;

void register_plic(plic_device_t *dev);
