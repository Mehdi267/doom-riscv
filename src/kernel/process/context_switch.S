/*
 * Projet PCSEA RISC-V
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 * Frédéric Pétrot <frederic.petrot@univ-grenoble-alpes.fr> - 2022
 * See license for license details.
 */

 //TODO : A chaque changement de contexte, il faut également changer le page directory pour que le processus qui va s'exécuter 
 //retrouve son espace virtuel, à savoir son code, sa pile et ses données. Lorsque le processus se termine, son mapping est détruit,
 // le page directory et le page table sont détruits, la mémoire physique est libérée. Le processus a libéré son espace.

 //TODO, ADD STACK PER PROCESS AND CHANGE THE SSCRATCH VALUE SO THAT WE CHANGE THE INTTEUPT STACK FOR EVERY PROCESS
 
 //Comment :  Le processus nouvellement activé disposera d'une nouvelle période de préemption aussi longue que possible (maximum 1 / SCHEDFREQ s) en perdant simplement le temps entre son activation et la précédente interruption d'horloge.
 // meaning what exactly

 //Comment : CONTEXTSWITCH USING CLOCK ???
.text 
.globl context_switch  
.align 2
context_switch:
	# Sauvegarde du contexte de l'ancien processus
	sd sp,    0(a0)
	sd ra,    8(a0)
	sd s0,   16(a0)
	sd s1,   24(a0)
	sd s2,   32(a0)
	sd s3,   40(a0)
	sd s4,   48(a0)
	sd s5,   56(a0)
	sd s6,   64(a0)
	sd s7,   72(a0)
	sd s8,   80(a0)
	sd s9,   88(a0)
	sd s10,  96(a0)
	sd s11, 104(a0)
    csrr t0, sscratch
    sd t0, 112(a0) 

	# Restauration du contexte
	ld sp,    0(a1)
	ld ra,    8(a1)
	ld s0,   16(a1)
	ld s1,   24(a1)
	ld s2,   32(a1)
	ld s3,   40(a1)
	ld s4,   48(a1)
	ld s5,   56(a1)
	ld s6,   64(a1)
	ld s7,   72(a1)
	ld s8,   80(a1)
	ld s9,   88(a1)
	ld s10,  96(a1)
	ld s11, 104(a1)
    //Sscratch
    ld t0, 112(a1) 
    csrs sscratch, t0
	
    # On passe la main au nouveau processus
	jr ra

