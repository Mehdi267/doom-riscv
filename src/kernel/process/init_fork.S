//This function will be used by the fork syscall, it will copy the registers 
//of the parent when the trap occured set the return value to zero and 
//then call the 
  .text
  .globl init_fork
  .align 2
init_fork:
  ld t0, 264(a0)    # sepc We pass the syscall
  add t0, t0, 4    # sepc
  csrw sepc, t0     // Set the value of sepc register
  ld ra, 8(a0)    # ra
  ld sp, 16(a0)   # sp
  ld gp, 24(a0)   # gp
  ld tp, 32(a0)   # tp
  ld t0, 40(a0)   # t0 
  ld t1, 48(a0)   # t1
  ld t2, 56(a0)   # t2
  ld s0, 64(a0)   # s0
  ld s1, 72(a0)   # s1
  ld a1, 88(a0)   # a1
  ld a2, 96(a0)   # a2
  ld a3, 104(a0)  # a3
  ld a4, 112(a0)  # a4
  ld a5, 120(a0)  # a5
  ld a6, 128(a0)  # a6
  ld a7, 136(a0)  # a7
  ld s2, 144(a0)  # s2
  ld s3, 152(a0)  # s3
  ld s4, 160(a0)  # s4
  ld s5, 168(a0)  # s5
  ld s6, 176(a0)  # s6
  ld s7, 184(a0)  # s7
  ld s8, 192(a0)  # s8
  ld s9, 200(a0)  # s9
  ld s10, 208(a0) # s10
  ld s11, 216(a0) # s11
  ld t3, 224(a0)  # t3
  ld t4, 232(a0)  # t4
  ld t5, 240(a0)  # t5
  ld t6, 248(a0)  # t6
  add a0, x0, x0  # a0 //We set this to zero in the fork process 
  sret