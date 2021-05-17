#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h> 
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <stdbool.h>
#include <signal.h>

#define SEM_KEY 0x713
#define MEM_KEY 0x1234
#define MAX_PIZZE 5

#define OVEN_DOOR_SEM 0	// starts as 1. How many chefs can acces the oven at once
#define OVEN_SEM 1	// starts as 5. How many available places are in the oven
#define TABLE_FREE_SEM 2	// starts as 5. How many free spaces at the table (for the chefs)
#define TABLE_BUSY_SEM 3	// starts as 0. How many pizzas at the table. (for the deliverers)
#define TABLE_ACCES_SEM 4 // starts as 1. How many people can acces the table at the same time 
#define SEM_COUNT 5

static volatile bool running = true;

/////////////////////// for shared memory
struct shared_data {
	int oven_counter;
	int table_counter;
	int oven[5];
	int table[5];
	struct timeval tv;
};


////////////////////// for semaphores
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
};

//////////////////////time
char *get_local_time(struct timeval *tv){
	char* buff = malloc(sizeof(char)*30);
	struct timeval tve;
	gettimeofday(&tve, NULL);
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
	int id;
	if ((id = shmget(MEM_KEY, 0, 0)) < 0) {
        perror("shmget: ");
        exit(1);
    }
    return id;
}

int get_semaphore(){
	int id;
	// 2nd argument is number of semaphores, if zero it gets the exisitng semaphore set
	// 3rd argument is the mode (IPC_CREAT creates the semaphore set if needed)
	if ((id = semget(SEM_KEY, 0, 0)) < 0) {
	    perror("get semget: ");
	}
	return id;
}

void change_sem(int semid, int semno, int diff){
	struct sembuf s = { semno, diff, SEM_UNDO};
	if(semop(semid, &s, 1) < 0){
		printf("could not chang the sem value %d\n", semno);
		perror("change sem: "); exit(14);
	}
}

int get_oven_status(int semid){
	return MAX_PIZZE - semctl(semid, OVEN_SEM, GETVAL);
}
int get_table_status(int semid){
	return semctl(semid, TABLE_BUSY_SEM, GETVAL);
}

#define get_to_table(code...) 								\
	change_sem(semid, TABLE_ACCES_SEM, -1);					\
	code													\
	change_sem(semid, TABLE_ACCES_SEM, 1);


void stop_worker(int signo){
	printf("Closing the worker %d\n", getpid());
	running = false;
}

void set_signal(){
	signal(SIGINT, stop_worker);
}