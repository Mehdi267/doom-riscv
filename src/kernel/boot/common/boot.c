/*
 * Projet PCSEA RISC-V
 *
 * Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 *
 * See license for license details.
 */

#include "stdlib.h"
#include "assert.h"
#include "riscv.h"

#include "bios/info.h"
#include "traps/trap.h"

/*
 * Prototypes externes
 */
// kernel/start.c
extern int kernel_start();

// Specific machine setup: kernel/boot/$(MACHINE)/setup.c
extern void arch_setup();


static void delegate_traps()
{

	/*
	 * Délégations de certaines interruptions et exceptions vers le mode Supervisor.
	 * A compléter au fur et à mesure du projet celon les besoins.
	 * Rien à faire ici dans un premier temps!
	 * CSR concernés: mideleg et medeleg.
	 */
}

static inline void setup_pmp(void) {
  /*
   * Désactivation de la protection de la mémoire physique (PMP).
   *
   * Configuration de la PMP pour donner  un accès complet à la mémoire.
   * CSR concernés: pmpaddr0 and pmpcfg0.
   */

  // Ignore the illegal-instruction trap if PMPs aren't supported.
  uintptr_t pmpc = PMP_NAPOT | PMP_R | PMP_W | PMP_X;
  uintptr_t pmpa = ((uintptr_t)1 << (__riscv_xlen == 32 ? 31 : 53)) - 1;
  __asm__ __volatile__("la t0, 1f\n\t"
                       "csrrw t0, mtvec, t0\n\t"
                       "csrw pmpaddr0, %1\n\t"
                       "csrw pmpcfg0, %0\n\t"
                       ".align 2\n\t"
                       "1: csrw mtvec, t0"
                       : : "r"(pmpc), "r"(pmpa) : "t0");
}

static inline void enter_supervisor_mode() {

  /* __asm__ __volatile__ ("csrw stap %0\n" */
  /*                       "csrw mstatus %0\n"); */
  csr_write(satp, 0); // virtual memory desactivated
  // global desactivation of interrupts
  uint64_t mstatus = csr_read(mstatus);
  csr_write(mstatus, mstatus ^ ((1<<1) | (1<<3))); //bits sie and mie are 1 and 3
  csr_write(mie, 0);

	// Il faut obligatoirement configurer la protection de la mémoire physique avant de passer en mode supervisor.
	setup_pmp();

	/*
	 * Configuration du mode à utiliser lors de l'instruction mret.
	 *
	 * CSR concernés: mepc et mstatus.
	 * Voir aussi riscv.h pour la macro mret().
	 */
  // changing to supervisor mode
  csr_write(mepc, kernel_start());
  mstatus = csr_read(mstatus);
  csr_write(mstatus, mstatus ^ (1<<12)); // writing 0 at bit 12 (0 indexed)
  csr_write(mstatus, 1<<11); // writing 1 at bit 11 | mpp = 01
  csr_write(sie, 1);
  mret();
}

/*
 * boot_riscv
 *
 * Cette fonction est appelée depuis crtm.S
 * A ce stade, seul le vecteur de trap machine mtvec a été configuré.
 * La pile en cours d'utilisation est la pile machine qui a été alouée dans crtm.S.
 * Cette pile est également utilisée pour traiter les traps machine.
 *
 * Le processeur est toujours en mode machine
 */
__attribute__((noreturn)) void boot_riscv()
{
	// Configuration des composants spécifiques à la machine (uart / htif, timer et interruptions externes).
	arch_setup();

	display_info_proc();

	// Délégations des interruptions et des exceptions
	delegate_traps();

	enter_supervisor_mode();
	exit(kernel_start());
	__builtin_unreachable();
}
