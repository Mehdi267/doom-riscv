



.text 
.globl process_call_wrapper_user  
.align 2
process_call_wrapper_user:
	move a0, s2
    sret
