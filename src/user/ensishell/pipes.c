#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "readcmd.h"


/*
This function is used to link multiple pipes, changing their file descriptors
and outputing the outcome of the pipe 
*/
void execute_multiple_pipe(char *** seq, int sequence_id, int background, char * in_file, char* out_file)
{   
	int file_d_table[2];
	pid_t process_1;
	pipe(file_d_table);
	process_1 = fork();
	switch (process_1)
	{
		case 0:
			dup2(file_d_table[0],0); 
			close(file_d_table[1]);
			close(file_d_table[0]);
			if (seq[sequence_id + 2]!=0){ 
                //if there are more pipes we contine with recursive calls
				execute_multiple_pipe(seq, sequence_id+1, background, in_file, out_file);
			}
			else {			
                //If this this the last command we run it direcly	
				run_process(seq[sequence_id +1], background, in_file, out_file, NO, END);
				exit(0);
			}			
			break;
		case -1:
			printf("fork problem, please review code and/or input :\n" ); break; 
		default:
            //In the parent process we rxecute the command that was provided in the parameters
			dup2(file_d_table[1],1);
			close(file_d_table[1]);
			close(file_d_table[0]);
			run_process(seq[sequence_id], background, in_file, out_file, NO, NO);
			exit(0);
			break;
	}
	return;
}

/*
This will execute a pipe with 2 commands and if ther are mroe it will call the 
execute_multiple_pipe to run the rest of the comamnds
*/
void execute_pipe(char *** seq, int sequence_id, int background, char * in_file, char* out_file)
{   
	int file_d_table[2];
	pid_t process_1;
	pid_t process_2;
	process_1 = fork();
	switch (process_1)
	{
	case 0:
		pipe(file_d_table);
		process_2 = fork();
		if (process_2 == 0)
		{
			dup2(file_d_table[0],0); 
			close(file_d_table[1]);
			close(file_d_table[0]);
			if (seq[sequence_id +2	]!=0){
				execute_multiple_pipe(seq, sequence_id+1, background, in_file, out_file);
			}
			else{			
				run_process(seq[sequence_id+1], background, in_file, out_file, NO, END);
				exit(0);
			}
		}
		dup2(file_d_table[1],1);
		close(file_d_table[0]);close(file_d_table[1]); 
		run_process(seq[sequence_id], background, in_file, out_file, START, NO);
		exit(0);
		break;
	case -1:
		printf("fork problem, please review code and/or input :\n" ); break; 
	default:
		waitpid_old(process_1, NULL);
		break;
	}
}
