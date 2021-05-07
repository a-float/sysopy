#include "common.h"

#define CHEF_PATH "chef.out"

void create_chefs(int n){
	char num[3];
	for(int i = 0; i < n; i++){
		if(fork() == 0){
			sprintf(num, "%d", i);
			execl(CHEF_PATH, CHEF_PATH, num, NULL);
		}
	}
}

int init_shared_memory(){
 	int id;
	if ((id = shmget(MEM_KEY, sizeof(struct shared_data), IPC_CREAT | 0666)) < 0) {
        perror("shmget: ");
        exit(1);
    }
    return id;
}

int init_semaphore(){
	int id;
	// 2nd argument is number of semaphores
	// 3rd argument is the mode (IPC_CREAT creates the semaphore set if needed)
	if ((id = semget(SEM_KEY, 1, 0666 | IPC_CREAT)) < 0) {
	    perror("semget: ");
	    exit(1);
	}
	union semun u;
	u.val = 1;
	// SETVAL is a macro to specify that you're setting the value of the semaphore to that specified by the union u
	if (semctl(id, 0, SETVAL, u) < 0) {
	    perror("semctl: ");
	    exit(1);
	}
	return id;
}

int main(int argc, char** argv){
	int N = 2;
	
	int semid = init_semaphore();   
    int shmid = init_shared_memory();
    
    struct shared_data *data = shmat(shmid, NULL, 0);

    for(int i = 0; i < MAX_PIZZE; i++){
    	data->oven[i] = -1;
    	data->table[i] = -1;
    }
    data->oven_counter = 0;
    data->table_counter = 0;

	create_chefs(N);
	for(int i = 0; i < 100; i++){
		sleep(1);
		// printf("oven   |  table\n");
		for(int i = 0; i < MAX_PIZZE; i++){
	    	printf("%d | %d\n", data->oven[i], data->table[i]);
	    }
	    printf("oven_counter = %d\n", data->oven_counter);
	}
	
	for(int i = 0; i < 2; i++){
		wait(NULL);
	}
	shmdt(data);
	shmctl(shmid, IPC_RMID, 0);
	// TODO remove the shared memory from the system?
	return 0;
}