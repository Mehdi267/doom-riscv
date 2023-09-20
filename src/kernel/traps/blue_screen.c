/*
 * Projet PCSEA RISC-V
 *
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 *
 * See license for license details.
 */

#include "stdio.h"
#include "stdlib.h"
#include "encoding.h"
#include "assert.h"
#include "trap.h"

#include "fs/fs.h"

static const char *land_names[4] = {
		"User",
		"Supervisor",
		"Hypervisor",
		"Machine"
};

/* Table 5.5, p. 94 of the RISC-V Privileged Architectures V1.12-draft document */
static const char *exception_names[16] = {
		"Instruction address misaligned",
		"Instruction access fault",
		"Illegal instruction",
		"Breakpoint",
		"Load address misaligned",
		"Load access fault",
		"Store/AMO address misaligned",
		"Store/AMO access fault",
		"Environment call from U-mode or VU-mode",
		"Environment call from HS-mode",
		"Environment call from VS-mode",
		"Environment call from M-mode",
		"Instruction page fault",
		"Load page fault",
		"Reserved",
		"Store/AMO page fault"
};

static const char *regnames[] = {
		"zero", "ra  ", "sp  ", "gp  ", "tp  ", "t0  ", "t1  ", "t2  ",
		"s0  ", "s1  ", "a0  ", "a1  ", "a2  ", "a3  ", "a4  ", "a5  ",
		"a6  ", "a7  ", "s2  ", "s3  ", "s4  ", "s5  ", "s6  ", "s7  ",
		"s8  ", "s9  ", "sA  ", "sB  ", "t3  ", "t4  ", "t5  ", "t6  "
};

static void dump_mtrap_frame(struct trap_frame *tf)
{
	puts("GPR :");
	uint64_t *regs = (uint64_t *) tf;
	for (int i = 0; i < 32; i += 4) {
		for (int j = 0; j < 4; j++)
			printf("%s %lx%c", regnames[i + j], regs[i + j], j < 3 ? ' ' : '\n');
	}

	puts("\nCSR :");
	printf("mepc    %lx\tmtval %lx\n", tf->mepc, tf->mtval);
	printf("mstatus %lx\tsatp  %lx\n", tf->mstatus, tf->satp);
}

void blue_screen(struct trap_frame *tf)
{
        /* Que ce soit bleu ! */
        printf("\x1b[97;104m");
	printf("\n********  RISC-V Exception N°%ld : %s  ********\n\n", tf->mcause, exception_names[tf->mcause]);

	dump_mtrap_frame(tf);

	uint8_t land = (uint8_t)((tf->mstatus & MSTATUS_MPP) >> 11);
	printf("\n%s from %s land at %lx\n", exception_names[tf->mcause], land_names[land], tf->mepc);
	printf("\033[0;m");
  if (land != 0){
    sync_all();
    assert(0);
  }
  // assert(0);
  // exit(-tf->mcause);
}