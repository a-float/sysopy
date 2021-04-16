#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#define MAX_COMP_COUNT 10
#define MAX_LINE_SIZE 120
#define MAX_TASK_COUNT 60
#define MAX_COMM_SIZE 30
#define MAX_COMM_COUNT 10
#define MAX_TASKS_LENGTH 10

typedef struct component{
	char *name;
	char **commands;
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
	res[len] = 0;
	// printf("from '%s' to '%s', start = %d, len = %d\n", src_cp, res, start, len);
	return res;
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
		// printf("'%s' is of size %d\n",line, read );
		line[read-1] = 0;
		if(is_reading_tasks == 0){
			char* token = strtok(line, "=");
			// printf("before '%s'\n", token);
			(*comps)[curr_comp].name = strcpy_trim(token);
			// printf("after '%s'\n", token);

			printf("'%s'\n",(*comps)[curr_comp].name);
			(*comps)[curr_comp].commands = calloc(sizeof(char*), MAX_COMM_COUNT);
			// printf("name %s has been saved %d %p\n", token, curr_comp, comps[curr_comp]);
			
			int curr_comm = 0;		
			while (token) {
			    // printf("token: '%s'\n", token);
			    token = strtok(NULL, "|");
			    if(token){
				    (*comps)[curr_comp].commands[curr_comm] = calloc(sizeof(char), MAX_COMM_SIZE);
				    strcpy((*comps)[curr_comp].commands[curr_comm], token);
					printf("'%s'\n",(*comps)[curr_comp].commands[curr_comm]);
				    curr_comm++;
				}
			}
			curr_comp++;
		}
		if(is_reading_tasks == 1 && strcmp(line, "\n") != 0){
			char* token = strtok(line, "|");
			// printf("token is '%s'\n",token );

			int j = 0;
			if(token){
				(*tasks)[curr_task].component_names = calloc(sizeof(char*), MAX_TASKS_LENGTH);
			}
			while(token){

				printf("token is '%s'\n",token );
		    	// printf("subtoken: '%s'\n", token);
			    (*tasks)[curr_task].component_names[j] = strcpy_trim(token);
			    // printf("saving %s to index %d in task no %d\n", token, j, curr_task);
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

	printf("'%s' '%s' '%s'\n", comps[0].name, comps[1].name, comps[2].name);
	printf("'%s' '%s' '%s'\n", tasks[0].component_names[1], tasks[1].component_names[0], tasks[2].component_names[0]);
	free_comps(&comps);
	free_tasks(&tasks);
	// char* t = strcpy_trim("  fuckem ");
	// printf("%s\n", t);
	// free(t);
	return 0;
}