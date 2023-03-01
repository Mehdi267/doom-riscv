/*
 * Projet PCSEA RISC-V
 * Mehdi Frikha
 * See license for license details.
 */
 
.text 
.globl context_switch  
.align 2
first_process_call:
# On se déplace directement dans le context du process,
# sans sauvegarde le context de l'ancienne execution
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