// See LICENSE for license details.

#include "stdint.h"
#include "drivers/auxval.h"
#include "drivers/poweroff.h"

#define SIFIVE_TEST_EXIT  0x5555

static volatile uint32_t *test;

static void virt_test_init()
{
	test = (uint32_t *)(void *)getauxval(VIRT_TEST_CTRL_ADDR);
}

static void virt_test_poweroff(int status)
{
	*test = (status << 16) | SIFIVE_TEST_EXIT;
	while (1);
}

poweroff_device_t poweroff_virt_test = {
	virt_test_init,
	virt_test_poweroff
};
