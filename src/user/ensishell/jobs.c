#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "readcmd.h"


/*
This executes the command jobs, thus showing jobs that are still in the background
*/
void output_process_bg()
{
    //We print out the running jobs in the background
	if (job_list == NULL)
		return;
	uint8_t process_number = 1; 
	job_bg_t* current_job_iterate = job_list;
	job_bg_t* next_job_iterate = NULL;
	while (current_job_iterate)
	{
		if (waitpid_old(current_job_iterate->job_id, NULL) != 0)
		{
			next_job_iterate = current_job_iterate->next_job;
			add_remove_job_to_the_background(current_job_iterate->job_name, current_job_iterate->job_id, 0);
			current_job_iterate = next_job_iterate;
		}
		else
		{
			printf("[%i] Id = %i , running : %s \n", process_number, current_job_iterate->job_id, current_job_iterate->job_name);
			current_job_iterate = current_job_iterate->next_job;
			process_number++;
		}
	}
}

/*
This function is used to add and remove for the linked list
that stores jobs in the background
*/
void add_remove_job_to_the_background(char* job_name, int job_id_process, bool add_or_remove)
{
	if (add_or_remove == 1)
	{
        //We add a job
		job_bg_t * job_bg_to_add;
		if (job_list == NULL)
		{
			job_list = (job_bg_t * ) malloc(sizeof(job_bg_t));
			job_bg_to_add = job_list;
		}
		else
		{
			job_bg_t * current_job_bg = job_list;
			job_bg_t * previous_job_bg;
			while (current_job_bg)
			{
				previous_job_bg = current_job_bg;
				current_job_bg = current_job_bg->next_job;
			}
			previous_job_bg->next_job = (job_bg_t * ) malloc(sizeof(job_bg_t));
			job_bg_to_add = previous_job_bg->next_job;
		}
		char * string_that_will_placed_in_job = (char * ) malloc(strlen(job_name)*sizeof(char));
		job_bg_to_add->job_id = job_id_process;
		strcpy(string_that_will_placed_in_job, job_name);
		job_bg_to_add->job_name = string_that_will_placed_in_job;
		job_bg_to_add->next_job = NULL;
	}
	else
	{
        //We remove a job
		job_bg_t * current_job_iterate = job_list;
		job_bg_t * previous_job_bg = NULL;
		while (current_job_iterate)
		{
			if (current_job_iterate->job_id == job_id_process)
				break;
			previous_job_bg = current_job_iterate;
			current_job_iterate = current_job_iterate->next_job;
		}
		free(current_job_iterate->job_name);
		if (previous_job_bg == NULL)
		{
			job_list =  current_job_iterate->next_job;
		}
		else if (current_job_iterate->next_job == NULL)
		{
			previous_job_bg->next_job = NULL;
		}
		else
		{
			previous_job_bg->next_job = current_job_iterate->next_job;
		}
		free(current_job_iterate);
	}

}
