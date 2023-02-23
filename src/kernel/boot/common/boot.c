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
#include "timer.h"
#include "drivers/splash.h"
#include "frame_dist.h"

extern void _start();
extern void test();
extern int tic;

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
	 * A compléter au fur et à mesure du projet selon les besoins.
	 * Rien à faire ici dans un premier temps!
	 * CSR concernés: mideleg et medeleg.
	 */

	//This function call delegated timer interrupt to the Supervisor mode 
  //instead of machine mode 
	csr_set(mideleg, SIE_STIE);
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

/**
* Cette fonction fera la configuration nécessaire pour le passage dans le mode
* de superviseur à partir du mode machine qui est le mode dans lequel on boot
* lors du lancement du processeur
*/
static inline void enter_supervisor_mode() {

  // Il faut obligatoirement configurer la protection de la mémoire physique
  // avant de passer en mode superviseur.
  setup_pmp();


  /*
    * Configuration du mode à utiliser lors de l'instruction mret.
    *
    * CSR concernés: mepc et mstatus.
    * Voir aussi riscv.h pour la macro mret().
  */

  // changing to supervisor mode
  //On met dans le registre mepc l'adresse de la méthode
  //qu'on exécutera en mode superviseur qui dans ce cas la méthode
  //kernel_start défini dans le fichier start.c
  csr_write(mepc, kernel_start);

  // L'objectif du code  suivant est dans le mettre dans la case MPP
  // du registre csr mstatus le niveau auquel on veut en aller après avoir traité
  // l'interruption auquel on est maintenant. Dans notre cas on veut passer du mode
  // actuel qui est le mode machine vers le mode superviseur qui est identifié avec
  // les bits suivants : 01

  csr_set(mstatus, MSTATUS_MPP_0);
  csr_clear(mstatus, MSTATUS_MPP_1);

  //On désactive les interruptions dans le mode superviseur
  csr_set(mstatus, MSTATUS_SIE);

  // Le passage au niveau mit dans le registre sera fait automatiquement avec l'instruction
  // mret qui changera le niveau suivant ce qui existe dans mpp
  mret();
}


/*
* boot_riscv
*
* Cette fonction est appelée depuis crtm.S
* A ce stade, seul le vecteur de trap machine mtvec a été configuré.
* La pile en cours d'utilisation est la pile machine qui a été allouée dans crtm.S.
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

  //On désactive pour le moment la mémoire virtuel,
  //éventuellement il faut revenir sur cela
  //et changer la valeur stockée dans le registre
  csr_write(satp, 0);

  //We disable machine mode interrupts
  csr_clear(mstatus, MSTATUS_MIE);

	//enables machine timer interrupts for the Supervisor mode
	csr_set(mie, SIE_STIE);
  
  //enables machine timer interrupts for the Supervisor mode
	csr_set(mip, MIP_STIP);

	//enables timer interrupts for the Supervisor mode
	csr_set(sie, SIE_STIE);

	//enables global Supervisor mode interrupts
	csr_set(sstatus, SSTATUS_SIE);


	//init timer to 0
	tic = 0;

	//set first timer interrupt
	set_supervisor_timer_interrupt(1000);

	init_frames();

	// for(int i = 0; i<10; i++){
		void *frame = get_frame();
		release_frame(frame);
	//}

	enter_supervisor_mode();
  //exit(kernel_start());
	__builtin_unreachable();
}
