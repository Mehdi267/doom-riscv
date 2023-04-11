#include "drivers/clint.h"

clint_device_t virt_clint = {
		.clk_freq = 10000000,
		.base_addr = 0x2000000,
		.supervisor_clint_available = false,
		.user_clint_available = false,
};
