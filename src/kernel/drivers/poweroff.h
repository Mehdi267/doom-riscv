// See LICENSE for license details.

#pragma once

typedef struct poweroff_device {
	void (*init)();
	void (*poweroff)(int);
} poweroff_device_t;


/*
 * poweroff_dev global variable
 * This variable is useful to call poweroff functions of the machine.
 */
extern poweroff_device_t *poweroff_dev;

/*
 * Poweroff drivers
 */
extern poweroff_device_t poweroff_none;
extern poweroff_device_t poweroff_htif;
extern poweroff_device_t poweroff_sifive_test;
extern poweroff_device_t poweroff_virt_test;
extern poweroff_device_t cep_poweroff;
extern poweroff_device_t poweroff_semihost;

void register_poweroff(poweroff_device_t *dev);
