



.text 
.globl process_call_wrapper  
.align 2
process_call_wrapper:
	  move a0, s2
    sret
