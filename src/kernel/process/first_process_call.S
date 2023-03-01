/*
 * Projet PCSEA RISC-V
 * Mehdi Frikha
 * See license for license details.
 */
 
.text 
.globl first_process_call  
.align 2
first_process_call:
# On se déplace directement dans le context du process,
# sans sauvegarde le context de l'ancienne execution
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
csrs sscratch, t0

//We need to activate intterupts at this level
# On passe la main au nouveau processus
jr ra