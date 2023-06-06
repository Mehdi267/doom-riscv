
.text 
.globl process_call_wrapper_kernel  
.align 2
process_call_wrapper_kernel:
	move a0, s2
    jalr s1
    j exit_process
