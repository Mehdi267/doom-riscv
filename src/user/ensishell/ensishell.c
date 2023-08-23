/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

#include "variante.h"
#include "readcmd.h"


#ifndef VARIANTE
#error "Variante non défini !!"
#endif

 job_bg_t *job_list = NULL;


/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

#if USE_GUILE == 1
#include <libguile.h>

void terminate(char *line) {
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
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

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


/*
This function is called in the case 
where we work with process that in the background
and when the finish they send a Sigchild signal to their parent 
calling this function
*/
void child_handler(int signalInt)
{
	//We need to first check that hte job is the list of the of jobs that are running in the background
	output_time_execution();
	sigaction(SIGCHLD, NULL, NULL);
}



/*
This function will take a sequence of characters and run them
and it also able to distingish between IN/OUT/BACKGROUND 
*/
void read_and_run(char ** seq, int background, char * in_file, char* out_file)
{   
    //There is a problem and it needs to be fixed (when the name of the program is not valid the process that was created does not get deleted)
    pid_t process_id;
    if (background)
	{
		struct sigaction sigactionChild;
		sigactionChild.sa_handler = child_handler;
		sigactionChild.sa_flags = SA_RESTART;
		sigaction(SIGCHLD, &sigactionChild, NULL);
	}
	process_id = fork();
	//printf("Here is the pid of the current process %i, and process_id is %i\n",getpid(), process_id);
    if (background && process_id !=0)
	{
		//Make so that is take not only seq[0] but all of the strings
		//printf("This is a backgroud function \n ");
		struct timeval storeTime;
		gettimeofday(&storeTime, NULL);
		add_remove_job_to_the_background(seq[0], process_id, storeTime.tv_sec,1);
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
        perror("fork problem, please review code and/or input :\n" ); break; 
    default:
        //printf("Father process detected \n");
		if (!background){
			waitpid(process_id, NULL, 0);		
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
		int fd_in = open(in_file, O_RDONLY);
		if (fd_in == -1) { perror("open  : " ); exit(EXIT_FAILURE);}
		dup2(fd_in, 0); 
		close(fd_in);
	}
	if (out_file && end)
	{
		fclose(fopen(out_file, "w"));   
		int fd_out = open( out_file, O_WRONLY);
		if (fd_out == -1) { perror("open  : " ); exit(EXIT_FAILURE);}
		ftruncate(fd_out,0);
		dup2(fd_out, 1); 
		close(fd_out);
	}
	execvp(seq[0], seq);
	execvp("false", seq);
}


/*
This function outputs the time that it took for the job that is the background to execute 
*/
void output_time_execution()
{
	job_bg_t * current_job_iterate = job_list;
	while (current_job_iterate)
	{   
		if (waitpid(current_job_iterate->job_id , NULL, WNOHANG) != 0)
		{
			int time_start = current_job_iterate->time_start;
			struct timeval storeTime;
			gettimeofday(&storeTime, NULL);
			printf("\n[Process] : %s that was exectuting in the background took %li to complete\n", current_job_iterate->job_name, storeTime.tv_sec-time_start);
			add_remove_job_to_the_background(current_job_iterate->job_name, current_job_iterate->job_id, 0, 0);
			break;
		}
		current_job_iterate = current_job_iterate->next_job;
	}
	return ;
}



int main() {
	printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	while (1) { 
		struct cmdline *l;
		char *line=0;
		int i, j;
		char *prompt = "ensishell>";

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || ! strncmp(line,"exit", 4)) {
			terminate(line);
		}

		if (!strncmp(line,"jobs", 4)) {
			output_process_bg();
			continue;
		}

#if USE_GNU_READLINE == 1
		add_history(line);
#endif


#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
                        continue;
                }
#endif

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


