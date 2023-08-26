/*****************************************************
 * Copyright Grégory Mounié 2008-2013                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

/*
 * changelog: add background, 2010, Grégory Mounié
 */

#ifndef __READCMD_H
#define __READCMD_H

#define START 1
#define END 1
#define NO 0

//Sycalls and local funtions
#include "../ulib/syscall.h"
#include "../ulib/ufunc.h"

/* If GNU Readline is not available, internal readline will be used*/
#include <stdbool.h>

/* Read a command line from input stream. Return null when input closed.
Display an error and call exit() in case of memory exhaustion. 
It frees also line and set it at NULL */
struct cmdline *parsecmd(char **line);


#if USE_GNU_READLINE == 0
/* Read a line from standard input and put it in a char[] */
char *readline(char *prompt);

#else
#include <readline/readline.h>
#include <readline/history.h>

#endif

/* Structure returned by parsecmd() */
struct cmdline {
	char *err;	/* If not null, it is an error message that should be
			   displayed. The other fields are null. */
	char *in;	/* If not null : name of file for input redirection. */
	char *out;	/* If not null : name of file for output redirection. */
        int   bg;       /* If set the command must run in background */ 
	char ***seq;	/* See comment below */
};

/* Field seq of struct cmdline :
A command line is a sequence of commands whose output is linked to the input
of the next command by a pipe. To describe such a structure :
A command is an array of strings (char **), whose last item is a null pointer.
A sequence is an array of commands (char ***), whose last item is a null
pointer.
When the user enters an empty line, seq[0] is NULL.
*/

/*
This datatype is used to save jobs the proporties of jobs
that are running in the background
and i saves the job id job name the time at which it started 
and next job in the linked list
*/
typedef struct job_bg {
	char * job_name;
	int job_id;
	int time_start;
	struct job_bg *next_job;
} job_bg_t;

/*
This global variable is a pointer to the linked list that 
stores all fo the jos running in the background
*/
extern job_bg_t *job_list;

/*
This enum is use to make the code more readable when 
working with dataypes
*/
typedef enum op { NONE,IN, OUT} Operation_Type;


/*
This function is used to add and remove for the linked list
that stores jobs in the background
*/
void add_remove_job_to_the_background(char* job_name, int job_id_process, bool add_or_remove);

/*
This function is called to run a process. 
*/
void run_process(char ** seq, int background, char * in_file, char* out_file, int start, int end);

/*
This function will take a sequence of characters and run them
and it also able to distingish between IN/OUT/BACKGROUND 
*/
void read_and_run(char ** seq, int background, char * in_file, char* out_file);

/*
This function is used to link multiple pipes, changing their file descriptors
and outputing the outcome of the pipe 
*/
void execute_multiple_pipe(char *** seq, int sequence_id, int background, char * in_file, char* out_file);

/*
This will execute a pipe with 2 commands and if ther are mroe it will call the 
execute_multiple_pipe to run the rest of the comamnds
*/
void execute_pipe(char *** seq, int sequence_id, int background, char * in_file, char* out_file);

/*
This executes the command jobs, thus showing jobs that are still in the background
*/
void output_process_bg();

/*
This function outputs the time that it took for the job that is the background to execute 
*/
void output_time_execution();

#endif
