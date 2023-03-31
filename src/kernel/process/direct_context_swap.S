/*
 * Projet PCSEA RISC-V
 * Mehdi Frikha
 * See license for license details.
 */

.text 
.globl direct_context_swap  
.align 2
direct_context_swap:
	# Restauration du contexte
	ld sp,    0(a0)
	ld ra,    8(a0)
	ld s0,   16(a0)
	ld s1,   24(a0)
	ld s2,   32(a0)
	ld s3,   40(a0)
	ld s4,   48(a0)
	ld s5,   56(a0)
	ld s6,   64(a0)
	ld s7,   72(a0)
	ld s8,   80(a0)
	ld s9,   88(a0)
	ld s10,  96(a0)
	ld s11, 104(a0)
  //Sscratch
  ld t0, 112(a0) 
  csrw sscratch, t0
  ld t0, 120(a0) 
  csrw sepc, t0
  ld t0, 128(a0) 
  csrw satp, t0

  jr ra

