#
# Projet PCSEA RISC-V
#
# Mathieu Barbe <mathieu@kolabnow.com> - 2019
#
# See license for license details.
#

.macro save_full_trap_frame
    # Récupération des GPR restant pour le blue screen
    sd zero, 0*8(sp)
    # sp a été placé dans sscratch
    csrr t0, sscratch
    sd t0, 2*8(sp)
    sd gp, 3*8(sp)
    sd tp, 4*8(sp)
    sd s0, 8*8(sp)
    sd s1, 9*8(sp)
    sd s2, 18*8(sp)
    sd s3, 19*8(sp)
    sd s4, 20*8(sp)
    sd s5, 21*8(sp)
    sd s6, 22*8(sp)
    sd s7, 23*8(sp)
    sd s8, 24*8(sp)
    sd s9, 25*8(sp)
    sd s10, 26*8(sp)
    sd s11, 27*8(sp)

    # Récupération des CSR
    csrr t1, sstatus
    sd t1, 32*8(sp)
    # sepc est dans a1
    sd a1, 33*8(sp)
    csrr t2, stval
    sd t2, 34*8(sp)
    # scause est dans a0
    sd a0, 35*8(sp)
    csrr t3, satp
    sd t3, 36*8(sp)
.endm

.globl strap_entry
.text
    .align 4
strap_entry:
    # sscratch contains the secured stack of the process
    # Using this command we swap the sscratch register and the sp register
    # meaning : sp<-sscratch //   sscratch<-sp
    csrrw sp, sscratch, sp 
    # On alloue la struct trap_frame sur la pile
    addi sp, sp, -37*8
    # Sauvegarde minimaliste du contexte (registre à la charge du caller)
    sd ra, 1*8(sp)
    sd t0, 5*8(sp)
    sd t1, 6*8(sp)
    sd t2, 7*8(sp)
    sd a0, 10*8(sp)
    sd a1, 11*8(sp)
    sd a2, 12*8(sp)
    sd a3, 13*8(sp)
    sd a4, 14*8(sp)
    sd a5, 15*8(sp)
    sd a6, 16*8(sp)
    sd a7, 17*8(sp)
    sd t3, 28*8(sp)
    sd t4, 29*8(sp)
    sd t5, 30*8(sp)
    sd t6, 31*8(sp)

    csrr    a0, scause
    csrr    a1, sepc
    mv a2, sp
    # Si exception hors ecall -> blue screen
    bltz a0, 1f
    andi t0, a0, 0xf
    li t1, 9
    beq t0, t1, 1f
    save_full_trap_frame
1:  jal strap_handler

    # Restauration des registres
    ld ra, 1*8(sp)
    ld t0, 5*8(sp)
    ld t1, 6*8(sp)
    ld t2, 7*8(sp)
    ld a0, 10*8(sp)
    ld a1, 11*8(sp)
    ld a2, 12*8(sp)
    ld a3, 13*8(sp)
    ld a4, 14*8(sp)
    ld a5, 15*8(sp)
    ld a6, 16*8(sp)
    ld a7, 17*8(sp)
    ld t3, 28*8(sp)
    ld t4, 29*8(sp)
    ld t5, 30*8(sp)
    ld t6, 31*8(sp)
    addi sp, sp, 37*8
    csrrw sp, sscratch, sp
    sret

.globl inc_sepc
.text
    .align 4
inc_sepc:
    csrr t0, sepc            // Read the current value of mstatus into t0
    addi t0, t0, 4          // Add 4 to the value in t0
    csrw sepc, t0           // Write the new value of t0 back to sepc
    ret
