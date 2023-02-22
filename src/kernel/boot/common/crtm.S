#
# Projet PCSEA RISC-V
#
# Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
# Mathieu Barbe <mathieu@kolabnow.com> - 2019
#
# See license for license details.
#

#include "constants.h"

# cf risc-v assembly programmer's handbook (pg 109 in risc-v spec)
.section .text.init,"ax",@progbits
.globl _start
_start:

    # Mise en place du vecteur de trap machine
    la      t0, mtrap_entry # load adress mtrap_entry in t0
    csrw    mtvec, t0 # write mtrap_entry in mtvec

    # Mise en place du vecteur de trap supervisor
    la      t0, strap_entry
    csrw    stvec, t0

    # Mise en place des piles (stacks) en fonction du hart (~core)
    csrr    t0, mhartid # mhardid is just number of the thread currectly running
    slli    t0, t0, STACK_SHIFT # shift left logic immediate  src, dest, 10: t0 = t0<<10
    la      sp, stacks + STACK_SIZE # sp = stacks + 1kb
    add     sp, sp, t0 # moving sp to the correct stack according to mhartid

    # On place dans mscratch le haut de la pile machine trap
    # elle sera réutilisée lors d'une interruption machine
    la      t1, mtrap_stacks + STACK_SIZE
    add     t0, t0, t1
    csrw    mscratch, t0
    add    t2,t0,x0 # used for debugging

    #On se redéplace par rapport à la pile 
    #du process qui est en cours d'execution
    csrr    t0, mhartid # mhardid is just number of the thread currectly running
    slli    t0, t0, STACK_SHIFT # shift left logic immediate  src, dest, 10: t0 = t0<<10

    # On place dans sscratch le haut de la pile supervisor trap
    # elle sera réutilisée lors d'une interruption superviseur
    la      t1, strap_stacks + STACK_SIZE
    add     t0, t0, t1
    csrw    sscratch, t0

    # On parque les harts autres que le premier pour booter sur un seul core.
    csrr    a0, mhartid # read cssr and store in a0
    bnez    a0, park # if (a0 != 0) goto park;


    j       boot_riscv

    # Endormissement du hart et réveil possible sur interruption IPI
park:
    wfi # wait for interruption
    j       park #jump to park

    .bss
    .align 4
    .global stacks
stacks:
    .skip STACK_SIZE * MAX_HARTS

    .align 4
    .global mtrap_stacks
mtrap_stacks:
    .skip STACK_SIZE * MAX_HARTS

    .align 4
    .global strap_stacks
strap_stacks:
    .skip STACK_SIZE * MAX_HARTS

