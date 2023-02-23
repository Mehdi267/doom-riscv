#include "riscv.h"
#include "stdio.h"

#include "drivers/clint.h"
#include "timer.h"
#include "drivers/auxval.h"

int tic;
/*
 * Set machine timer
 *
 * Cette fonction configure la prochaine interruption timer machine delta ms dans le futur.
 *
 *@param delta_ms : réglage de la prochaine interruption àtimer machine  cur + delta ms.
 */
void set_machine_timer_interrupt(uint64_t delta_ms)
{

	/*
	 * Configuration du clint
	 * L'adresse et la fréquence du clint est dépendante de la plateforme,
	 * La variable globale clint_dev de type clint_device_t nous renseigne sur ces paramettres:
	 * - clint_dev->clk_freq indique la fréquence d'incrémentations du registre time.
	 * - clint_dev->base_addr indique l'adresse où est mapé la base des registres du clint.
	 *
	 * Les macros get_mtime() et set_mtimecmp(x) de "drivers/clint.h" sont données pour lire et écrire les registres de ce composant.
	 */

	uint64_t interrupt_time = get_mtime() + (delta_ms/1000) * clint_dev->clk_freq; // get time of next interrupt
	set_mtimecmp(interrupt_time); // lorsque mtime >= mtimecmp, une interruption est générée
}


void set_supervisor_timer_interrupt(uint64_t delta_ms)
{
	uint64_t interrupt_time = get_stime() + (delta_ms/1000) * clint_dev->clk_freq; // get time of next interrupt
	set_stimecmp(interrupt_time); // lorsque stime >= stimecmp, une interruption est générée
}

void handle_mtimer_interrupt()
{
	puts("test machine!\n");
	printf("Tic \n");
	set_machine_timer_interrupt(TIC_PER); // this fills the role of ack
	tic++;
}

void handle_stimer_interrupt()
{
	printf("Tic \n");
	set_supervisor_timer_interrupt(TIC_PER); // this fills the role of ack
	tic++;
}