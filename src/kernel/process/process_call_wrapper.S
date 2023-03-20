



.text 
.globl process_call_wrapper  
.align 2
process_call_wrapper:
	move a0, s2
	# sd t0,0(sp)
	# addi sp,sp,-4
	# jr s1
	jalr s1
	j exit_process
    
