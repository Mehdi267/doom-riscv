



.text 
.globl process_call_wrapper  
.align 2
process_call_wrapper:
	move a0, s2
	jalr s1
	j exit_process
    
