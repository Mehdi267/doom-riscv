/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "readcmd.h"
job_bg_t *job_list = NULL;


void terminate(char *line) {
	if (line)
	  free(line);
	printf("exit\n");
	exit(0);
}

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	 * identically to the standard execution scheme:
	 * parsecmd, then fork+execvp, for a single command.
	 * pipe and i/o redirection are not required.
	 */
	struct cmdline *l;
	l = parsecmd( & line);

	/* If input stream closed, normal termination */
	if (!l) {
		terminate(0);
	}
		
	if (l->err) {
		/* Syntax error, read another command */
		printf("error: %s\n", l->err);
	}
	
	/* Display each command of the pipe */
	for (int i=0; l->seq[i]!=0; i++) {
		char **cmd = l->seq[i];
		printf("seq[%d]: ", i);
					for (int j=0; cmd[j]!=0; j++) {
							printf("'%s' ", cmd[j]);
					}
	printf("\n");
	read_and_run(cmd, l->bg,l->in, l->out);
}
	/* Remove this line when using parsecmd as it will free it */
	free(line);
	
	return 0;
}


/*
This function will take a sequence of characters and run them
and it also able to distingish between IN/OUT/BACKGROUND 
*/
void read_and_run(char ** seq, int background, char * in_file, char* out_file)
{   
  //There is a problem and it needs to be fixed (when the name of the program is not valid the process that was created does not get deleted)
  pid_t process_id;
	process_id = fork();
	//printf("Here is the pid of the current process %i, and process_id is %i\n",getpid(), process_id);
  if (background && process_id !=0)
	{
		add_remove_job_to_the_background(seq[0], process_id, 1);
	}
	switch (process_id)
  {
    case 0:
	    //There is a problem and it needs to be fixed (when the name of the program is not valid the process that was created does not get deleted)
		//printf("Child process running \n");
		run_process(seq, background, in_file, out_file, START, END);
		exit(0);
		break;
    case -1:
        printf("fork problem, please review code and/or input :\n" ); break; 
    default:
        //printf("Father process detected \n");
		if (!background){
			waitpid_old(process_id, NULL);		
		}
		break;
  }
}

/*
This function is called to run a process. 
*/
void run_process(char ** seq, int background, char * in_file, char* out_file, int start, int end)
{
	if (in_file && start)
	{
		int fd_in = open(in_file, O_RDONLY, 0);
		if (fd_in == -1) { printf("open in failed: " ); exit(1);}
		if(dup2(fd_in, 0)<0){printf("dup in failed: " ); exit(1);}  
		close(fd_in);
	}
	if (out_file && end)
	{
		int fd_out = open(out_file, O_WRONLY | O_TRUNC | O_CREAT, 0);
		if (fd_out == -1) { printf("open out failed: " ); exit(1);}
		if(dup2(fd_out, 1)<0){printf("dup out failed: " ); exit(1);} 
		close(fd_out);
	}
	char* base_file = "/bin/";
  int len = strlen(base_file) + strlen(seq[0]) + 1;
  char* prog = malloc(len);
  memcpy(prog, base_file, strlen(base_file));
  memcpy(prog + strlen(base_file),
         seq[0], 
         strlen(seq[0]));
  prog[len-1] = '\0';
  printf("Execting the program %s\n", prog);
  execve(prog, seq, NULL);
}



int main() {
  //We check if these files were opened before
  //stdin
  assert(open("/dev/terminal", O_RDONLY, 0) != -1);
  //stdout
  assert(open("/dev/terminal", O_WRONLY, 0) != -1);
  //stderr
  assert(dup2(1, 2) != -1);
  // assert(open("/dev/terminal", O_WRONLY, 0) != -1);
	while (1) { 
		struct cmdline *l;
		char *line=0;
		int i, j;
    #define CURR_DIR_SIZE 50
    #define PROMPT_SIZE 90
    char prompt_final[PROMPT_SIZE];
		char *prompt = "\033[0;32mensishell\033[0;0m:";
    memcpy(prompt_final, prompt, strlen(prompt));
    int pos = strlen(prompt); 
    char current_dir[CURR_DIR_SIZE];
    if (getcwd(current_dir, CURR_DIR_SIZE) != 0){
      memcpy(prompt_final+pos, current_dir, strlen(current_dir));
      pos += strlen(current_dir);
    }
    char *term = "#>";
    memcpy(prompt_final+pos, term, strlen(term));
    pos += strlen(term);
		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt_final);
		if (line == 0 || !strncmp(line,"exit", 4)) {
			terminate(line);
		}
		if (!strncmp(line,"jobs", 4)) {
			output_process_bg();
			continue;
		}
    if (!strncmp(line,"cd", 2)) {
      change_directory((char*)line+2);
			continue;
		}
		/* parsecmd free line and set it up to 0 */
		l = parsecmd( & line);

		/* If input stream closed, normal termination */
		if (!l) {  
			terminate(0);
		}
		
		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);
		if (l->bg) printf("background (&)\n");
		

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
                        for (j=0; cmd[j]!=0; j++) {
                                printf("'%s' ", cmd[j]);
                        }
			printf("\n");
			if (l->seq[i+1]!=0)
			{
				//In this case we detect a pipe we call this function
				execute_pipe(l->seq, i,  l->bg,l->in, l->out);
				break;
			}
			else if (l->in || l->out)
			{
				read_and_run(cmd, l->bg,l->in, l->out);
				continue;
			}
			else
			{
				read_and_run(cmd, l->bg,l->in, l->out);
			}
		}
	}

}


