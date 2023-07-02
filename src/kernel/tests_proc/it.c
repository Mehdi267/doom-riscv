#include "it.h"

#include "riscv.h"
#include "encoding.h"

// iteration with a nop to catch the pending irq
void test_it()
{
	uint64_t tmp_status;
	tmp_status = csr_read(sstatus);
	csr_set(sstatus, MSTATUS_SIE);
	__asm__ __volatile__("nop");
	csr_write(sstatus, tmp_status);
}
