#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_COMP_COUNT 10
#define MAX_LINE_SIZE 120
#define MAX_TASK_COUNT 60
#define MAX_COMM_COUNT 10
#define MAX_ARG_SIZE 20
#define MAX_TASKS_LENGTH 10
#define MAX_ARG_COUNT 7

typedef struct component{
	char *name;
	char ***commands;
} component;

typedef struct task{
	char **component_names;
} task;

//doesnt support strings with separate words
//allocs and returns a pointer to new string
char* strcpy_trim(char* src){
	int found_letter = 0;
	int start = 0, len = 0;
	char* src_cp = src;
	while(*src != 0){
		// printf("%c\n", *src);
		if(found_letter == 0 && *src == ' '){
			//trim front spaces
			start++;
		}
		else if(*src != ' '){
			//reading the string itself
			found_letter = 1;
			len++;
		}
		src++;
	}
	char* res = calloc(sizeof(char),len+1);
	for(int i = 0; i < len; i++){
		res[i] = src_cp[start+i];
	}
	return res;
	return NULL;
}

char** get_args_from_command(const char* original_command){
	char* command = strdup(original_command);
	char** args = calloc(sizeof(char*), MAX_ARG_COUNT);
	char* token = strtok(command, " ");
	int i = 0;
	while(token){
		args[i] = strdup(token);
		token = strtok(NULL, " ");
		i++;
	}
	free(command);
	return args;
	return NULL;
}

int load_components(FILE* fp, component** comps, task** tasks){
	size_t buff_size = MAX_LINE_SIZE;
	char *line = malloc(buff_size*sizeof(char));
	int read;
	int is_reading_tasks = 0;
	int curr_comp = 0;
	int curr_task = 0;
	while((read = getline(&line, &buff_size, fp)) >= 0){
		if(strcmp(line, "\n") == 0){
			is_reading_tasks = 1;
		}
		if(line[read-1] == '\n')line[read-1]=0;
		if(is_reading_tasks == 0){
			char* saveptr;
			char* token = strtok_r(line, "=", &saveptr);
			(*comps)[curr_comp].name = strcpy_trim(token);
			(*comps)[curr_comp].commands = calloc(sizeof(char*), MAX_COMM_COUNT);
			
			int curr_comm = 0;		
			while (token) {
			    token = strtok_r(NULL, "|", &saveptr);
			    if(token != NULL){
				    (*comps)[curr_comp].commands[curr_comm] = get_args_from_command(token);
				    curr_comm++;
				}
			}
			curr_comp++;
		}
		if(is_reading_tasks == 1 && strcmp(line, "\n") != 0){
			char* token = strtok(line, "|");
			int j = 0;
			if(token){
				(*tasks)[curr_task].component_names = calloc(sizeof(char*), MAX_TASKS_LENGTH);
			}
			while(token){
			    (*tasks)[curr_task].component_names[j] = strcpy_trim(token);
				j++;
				token = strtok(NULL, "|");
				if(token == NULL)curr_task++;
			}
		}
	}
	free(line);
	return 0;
}

void free_comps(component **comps_p){
	if(comps_p == NULL)return;

	component* comps = *comps_p;
	for(int i = 0; i < MAX_COMP_COUNT; i++){

		if(comps[i].commands == NULL)break;	// all components are allocated, but only the few first have commands
		for(int j = 0; j < MAX_COMM_COUNT; j++){
			
			if(comps[i].commands[j] != NULL){
				for(int k = 0; k < MAX_ARG_COUNT; k++){
					if(comps[i].commands[j][k] == NULL)break; //no more args
					free(comps[i].commands[j][k]);
					comps[i].commands[j][k] = NULL;
				}
				free(comps[i].commands[j]);
				comps[i].commands[j] = NULL;
			}
		}
		free(comps[i].commands);
		free(comps[i].name);
		comps[i].commands = NULL;
	}
	free(comps);
	comps_p = NULL;
}

void free_tasks(task **tasks_p){
	if(tasks_p == NULL)return;
	task* tasks = *tasks_p;
	for(int i = 0; i < MAX_TASKS_LENGTH; i++){

		if(tasks[i].component_names == NULL)break;
		for(int j = 0; j < MAX_TASKS_LENGTH; j++){

			if(tasks[i].component_names[j] != NULL){
				free(tasks[i].component_names[j]);
				tasks[i].component_names[j] = NULL;
			}
		}
		free(tasks[i].component_names);
		tasks[i].component_names = NULL;
	}
	free(tasks);
	tasks_p = NULL;
}

component* get_component_by_name(char* name, component *comps){
	for(int i = 0; i < MAX_COMP_COUNT; i++){
		if(comps[i].name == NULL)return NULL; //there is no more components
		if(strcmp(comps[i].name, name) == 0)return &comps[i];
	}
	return NULL; //component not found
}

//sets the in of the component chain to out_pipe[0], and its out to out_pipe[1]
void execute_component(component comp, int* out_pipe){
	int curr[2];
	int prev;
	prev = out_pipe[0];
	for(int i = 0; i < MAX_COMM_COUNT; i++){
		if(comp.commands[i] == NULL)break; //no more commands
		pipe(curr);
		if(fork() == 0){
			close(curr[0]);
			char** args = comp.commands[i];
			dup2(prev, STDIN_FILENO);
			dup2(curr[1], STDOUT_FILENO);
			execvp(args[0], args);
			exit(1);
		}
		close(curr[1]);
		prev = curr[0];
	}
	out_pipe[0] = prev;
}

void execute_task(task t, component *comps){
	component* cp;
	int curr[2];
	pipe(curr);
	close(curr[1]);
	for(int i = 0; i < MAX_COMM_COUNT; i++){
		if(t.component_names[i] == NULL)break; //no more commands
		cp = get_component_by_name(t.component_names[i], comps);
		if(cp == NULL)return; //something went wrong
		execute_component(*cp, curr);
	}
	char buff[50];
	int read_count;
	while((read_count = read(curr[0], buff, sizeof(buff))) > 0){
		write(1, buff, read_count);
	}
	while(wait(NULL) > 0);
	// printf("A child has exited.\n");
}

int main(int argc, char **argv) {
	if(argc < 2){
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(1);
	}
	FILE* fp = fopen(argv[1], "r");
	if(fp == NULL){
		fprintf(stderr, "Could not open the file %s\n", argv[1]);
		exit(1);
	}
	component* comps = calloc(sizeof(component), MAX_COMP_COUNT);
	
	task* tasks = calloc(sizeof(task), MAX_TASK_COUNT);
	load_components(fp, &comps, &tasks);
	fclose(fp);


	// executing the commands
	for(int i = 0; i < MAX_TASK_COUNT; i++){
		if(tasks[i].component_names == NULL)break; //the rest of the tasks is empty
		printf("Task no %d:\n", i);
		execute_task(tasks[i], comps);
	}

	free_comps(&comps);
	free_tasks(&tasks);
	return 0;
}