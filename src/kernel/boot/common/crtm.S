#
# Projet PCSEA RISC-V
#
# Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
# Mathieu Barbe <mathieu@kolabnow.com> - 2019
#
# See license for license details.
#

#include "constants.h"

// cf risc-v assembly programmer's handbook (pg 109 in risc-v spec)
.section .text.init,"ax",@progbits
.globl _start
_start:
    # Mise en place du vecteur de trap machine
    la      t0, mtrap_entry // load adress mtrap_entry in t0
    csrw    mtvec, t0 // write mtrap_entry in mtvec

    # Mise en place des piles (stacks) en fonction du hart (~core)
    csrr    t0, mhartid // mhardid is just number of the processeur running the code
    slli    t0, t0, STACK_SHIFT // shift left logic immediate  src, dest, 10: t0 = t0<<10
    la      sp, stacks + STACK_SIZE // sp = stacks + 1kb
    add     sp, sp, t0 // moving sp to the correct stack according to mhartid

    # On place dans mscratch le haut de la pile machine trap
    # elle sera réutilisée lors d'une interruption machine
    la      t1, mtrap_stacks + STACK_SIZE
    add     t0, t0, t1
    csrw mscratch, t0

    # init of 32 register to 0
    lui x0, 0
    lui x1, 0
    lui x2, 0
    lui x3, 0
    lui x4, 0
    lui x5, 0
    lui x6, 0
    lui x7, 0
    lui x8, 0
    lui x9, 0
    lui x10, 0
    lui x11, 0
    lui x12, 0
    lui x13, 0
    lui x14, 0
    lui x15, 0
    lui x16, 0
    lui x17, 0
    lui x18, 0
    lui x19, 0
    lui x20, 0
    lui x21, 0
    lui x22, 0
    lui x23, 0
    lui x24, 0
    lui x25, 0
    lui x26, 0
    lui x27, 0
    lui x28, 0
    lui x29, 0
    lui x30, 0
    lui x31, 0

    # On parque les harts autres que le premier pour booter sur un seul core.
    csrr    a0, mhartid // read cssr and store in a0
    bnez    a0, park // if (a0 != 0) goto park;


    j       boot_riscv

    # Endormissement du hart et réveil possible sur interruption IPI
park:
    wfi // wait for interruption
    j       park //jump to park

    .bss
    .align 4
    .global stacks
stacks:
    .skip STACK_SIZE * MAX_HARTS

    .align 4
    .global mtrap_stacks
mtrap_stacks:
    .skip STACK_SIZE * MAX_HARTS
