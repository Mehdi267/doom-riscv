/*
 * Projet PCSEA RISC-V
 * Mathieu Barbe <mathieu@kolabnow.com> - 2019
 * Frédéric Pétrot <frederic.petrot@univ-grenoble-alpes.fr> - 2022
 * See license for license details.
 */

  .text
  .globl context_switch
  .align 2
context_switch:
  # Save the context of the old process
  sd sp,    0(a0)   // Save the stack pointer
  sd ra,    8(a0)   // Save the return address
  sd s0,   16(a0)   // Save register s0
  sd s1,   24(a0)   // Save register s1
  sd s2,   32(a0)   // Save register s2
  sd s3,   40(a0)   // Save register s3
  sd s4,   48(a0)   // Save register s4
  sd s5,   56(a0)   // Save register s5
  sd s6,   64(a0)   // Save register s6
  sd s7,   72(a0)   // Save register s7
  sd s8,   80(a0)   // Save register s8
  sd s9,   88(a0)   // Save register s9
  sd s10,  96(a0)   // Save register s10
  sd s11, 104(a0)   // Save register s11
  csrr t0, sscratch  // Get the value of sscratch register (secured stack of the process)
  sd t0, 112(a0)    // Save the value of sscratch register
  csrr t0, sepc      // Get the value of sepc register (virtual address of the instruction that encountered the exception)
  sd t0, 120(a0)    // Save the value of sepc register
  csrr t0, satp      // Get the value of satp register (pointer to the level 2 page table associated with the old process)
  sd t0, 128(a0)    // Save the value of satp register
  # Restore the context
  ld sp,    0(a1)   // Restore the stack pointer
  ld ra,    8(a1)   // Restore the return address
  ld s0,   16(a1)   // Restore register s0
  ld s1,   24(a1)   // Restore register s1
  ld s2,   32(a1)   // Restore register s2
  ld s3,   40(a1)   // Restore register s3
  ld s4,   48(a1)   // Restore register s4
  ld s5,   56(a1)   // Restore register s5
  ld s6,   64(a1)   // Restore register s6
  ld s7,   72(a1)   // Restore register s7
  ld s8,   80(a1)   // Restore register s8
  ld s9,   88(a1)   // Restore register s9
  ld s10,  96(a1)   // Restore register s10
  ld s11, 104(a1)   // Restore register s11
  ld t0, 112(a1)    // Load the value of sscratch register
  csrw sscratch, t0 // Set the value of sscratch register
  ld t0, 120(a1)    // Load the value of sepc register
  csrw sepc, t0     // Set the value of sepc register
  ld t0, 128(a1)    // Load the value of satp register
  csrw satp, t0     // Set the value of satp register
  jr ra             // Jump to the address pointed by register ra
  ret               // Equivalent to jr ra, but clearer
  // jr ra          // Jump to the address pointed by register ra
  // When we do a jump and link, we jump to the address pointed by ra