#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdbool.h>
#include <signal.h>

#define SEM_NAME "/MY_SEM"
#define MEM_NAME "/MY_MEMORY"

#define MAX_PIZZE 5

#define OVEN_DOOR_SEM 0	// starts as 1. How many chefs can acces the oven at once
#define OVEN_SEM 1	// starts as 5. How many available places are in the oven
#define TABLE_FREE_SEM 2	// starts as 5. How many free spaces at the table (for the chefs)
#define TABLE_BUSY_SEM 3	// starts as 0. How many pizzas at the table. (for the deliverers)
#define TABLE_ACCES_SEM 4 // starts as 1. How many people can acces the table at the same time 
#define SEM_COUNT 5

static volatile bool running = true;

char *names[] = {"/SEM1","/SEM2","/SEM3","/SEM4","/SEM5"};
/////////////////////// for shared memory
struct shared_data {
	int oven_counter;
	int table_counter;
	int oven[5];
	int table[5];
	struct timeval tv;
};

//////////////////////time
char *get_local_time(struct timeval *tv){
	char* buff = malloc(sizeof(char)*30);
	struct timeval tve;
	gettimeofday(&tve,NULL);
	sprintf(buff, "%lds %2.ldms", tve.tv_sec - tv->tv_sec, tve.tv_usec - tv->tv_usec);
	return buff;
}

void print_timestamp(int id, struct timeval *tv){
	char* time = get_local_time(tv);
	printf("(id:%d %s) ", id, time);
	free(time);
}

#define print_oven													\
	for(int i = 0; i < MAX_PIZZE; i++){								\
    	printf("%d | %d\n", data->oven[i], data->table[i]);			\
    }																
    // printf("oven_counter = %d\n", data->oven_counter);				

int get_shared_memory(){
	int fd;
	fd = shm_open(MEM_NAME, O_RDWR, 0666);
	if (fd == -1){
		perror("create shm_open: ");
		exit(11);
	}
    return fd;
}

sem_t **get_semaphores(){
	// unsigned int vals[] = {1, MAX_PIZZE, MAX_PIZZE, 0, 1}; // 1 space at the oven, full free oven, empty table, accesible table
	sem_t **sems = calloc(sizeof(sem_t), SEM_COUNT);
	for(int i = 0; i < SEM_COUNT; i++){
		sems[i] = sem_open(names[i], 0);
		if(sems[i] == SEM_FAILED){
			perror("get sem_open: ");
			exit(17);
		}
	}
	return sems;
}

int get_oven_status(sem_t **sems){
	int value;
	sem_getvalue(sems[OVEN_SEM], &value);
	return MAX_PIZZE - value;
}
int get_table_status(sem_t **sems){
	int value;
	sem_getvalue(sems[TABLE_BUSY_SEM], &value);
	return value;
}

#define get_to_table(code...) 								\
	sem_wait(sems[TABLE_ACCES_SEM]);						\
	code													\
	sem_post(sems[TABLE_ACCES_SEM]);

void stop_worker(int signo){
	printf("Closing the worker %d\n", getpid());
	running = false;
	exit(99);
}

void set_signal(){
	signal(SIGINT, stop_worker);
}