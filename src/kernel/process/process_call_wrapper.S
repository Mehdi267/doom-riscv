



.text 
.globl process_call_wrapper  
.align 2
process_call_wrapper:
	addi a0, s2, 0
	jalr s1 
	j exit_process