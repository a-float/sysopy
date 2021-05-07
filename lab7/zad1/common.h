#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h> 
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <time.h>

#define SEM_KEY 0x713
#define MEM_KEY 0x1234
#define MAX_PIZZE 5

/////////////////////// for shared memory
struct shared_data {
	int oven_counter;
	int table_counter;
	int oven[5];
	int table[5];
};


////////////////////// for semaphores
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
};

struct sembuf p = { 0, -1, SEM_UNDO}; // semwait
struct sembuf v = { 0, +1, SEM_UNDO}; // semsignal

//////////////////////time
char *get_local_time(){
	char* buff = malloc(sizeof(char)*30);

	struct tm *tmp;
	time_t curtime;

	time(&curtime);
	tmp = localtime(&curtime);

	strftime(buff, 30, "%H:%M:%S", tmp);
	return buff;
}