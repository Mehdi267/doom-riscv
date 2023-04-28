#include "process/process.h"
#include "riscv.h"
#include "stdio.h"

#include "drivers/clint.h"
#include "timer.h"
#include "drivers/auxval.h"
#include "process/scheduler.h" // for scheduler car
#include "process/helperfunc.h" //to use helper functions
#include "process/timer_api.h" // to exploit the counter variable
#include <stdbool.h>
#include <stdint.h>

bool first_call = true;

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

	uint64_t interrupt_time = get_mtime() + (delta_ms * clint_dev->clk_freq )/1000; // get time of next interrupt
	set_mtimecmp(interrupt_time); // lorsque mtime >= mtimecmp, une interruption est générée
    return ;
}


void set_supervisor_timer_interrupt(uint64_t delta_ms)
{
	uint64_t interrupt_time = get_stime() + ( delta_ms * clint_dev->clk_freq )/1000; // get time of next interrupt
	set_stimecmp(interrupt_time); // lorsque stime >= stimecmp, une interruption est générée
    return ;
}

void handle_mtimer_interrupt()
{
	//Machine cep
	#ifndef VIRTMACHINE
		printf("Tic machine \n");
		set_machine_timer_interrupt(TIC_PER); // this fills the role of ack
		counter++;
		return ;
	//Machine virt
	#else
		//inspired for the xv6 project
		//We force a timer intterupt that will have 
		//to go through the kernel mode 
		//Timer interupts must come from user mode, if it is not the case we return 
		//directly to the running execution
		debug_print_no_arg("Inside machine mode time handler\n");
		if (((csr_read(mstatus) & MSTATUS_MPP_0) != 0) && !first_call){
			debug_print_no_arg("Int comming from supervisor mode and " 
					"we are not in the first call \n");
			set_machine_timer_interrupt(TIC_PER);
			return;
		}
		first_call = false;
		debug_print_no_arg("-------------before interrupt-------------\n");
		csr_write(sip, 2);
		debug_print_no_arg("-------------testing------------\n");
		set_machine_timer_interrupt(TIC_PER);
		return;
	#endif
}

void handle_stimer_interrupt()
{
	//Machine cep
	#ifndef VIRTMACHINE
		// debug_print_no_arg("--Inside timer interreupt handler-- \n"); 
		#ifdef KERNEL_PROCESSES_ON
			set_supervisor_interrupts(false);
		#endif
		counter++;
		set_supervisor_timer_interrupt(TIC_PER); 
		scheduler();
	//Machine virt
	#else
		debug_print_no_arg("--Called from m mode"  
					"Inside timer interreupt handler supervisor -- \n"); 
		counter++;
		scheduler();
		set_supervisor_interrupts(true);
	#endif
  return;
}
