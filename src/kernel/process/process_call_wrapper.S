



.text 
.globl process_call_wrapper  
.align 2
process_call_wrapper:
	move a0, s2
	# csr_write(mepc, kernel_start);
	csrw sepc, s1
	sret 
	j exit_process