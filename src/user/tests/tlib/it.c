/*******************************************************************************
 * Unmask interrupts for those who are working in kernel mode
 ******************************************************************************/

#define csr_set(csr, value)													\
({																			\
	register unsigned long int __v = (uint64_t)(value);								\
	__asm__ __volatile__ ("csrs " #csr ", %0" : : "rK" (__v) : "memory");	\
})

// value is a bit mask that specifies bit position to be cleared (high bit)
#define csr_clear(csr, value)												\
({																			\
	register unsigned long int __v = (uint64_t)(value);								\
	__asm__ __volatile__ ("csrc " #csr ", %0" : : "rK" (__v) : "memory");	\
})

void test_it(void)
{
	#ifndef USER_SPACE
      csr_set(sstatus, 2); // sstatus.SIE = 1
      // Instruction wfi, Wait for interrupt
      __asm__ __volatile__ ("wfi" ::: "memory");
      csr_clear(sstatus, 2); // sstatus.SIE = 0
	#endif
}
