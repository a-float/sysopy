#include "common.h"

#define CHEF_PATH "chef.out"
#define DELIVERER_PATH "deliverer.out"

void stop_program(int signo){
	printf("Stopping the main program\n");
	running = false;
}

void create_workers(int n, const char* path, int startid, pid_t *pids){
	char num[3];
	for(int i = 0; i < n; i++){
		pid_t pid = fork();
		if(pid == 0){
			sprintf(num, "%d", startid+i);
			execl(path, path, num, NULL);
		}
		pids[startid+i] = pid;
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
	if ((id = semget(SEM_KEY, SEM_COUNT, 0666 | IPC_CREAT )) < 0) { //creates a set of 3 semaphores
	    perror("create semget: ");
	    exit(1);
	}
	union semun u;
	ushort arr[] = {1, MAX_PIZZE, MAX_PIZZE, 0, 1}; // 1 space at the oven, full free oven, empty table, accesible table
	u.array = arr;
	//SETALL sets all sempahores according to the semun array values
	if (semctl(id, -999, SETALL, u) < 0) {
	    perror("setall semctl: ");
	    exit(1);
	}
	return id;
}

int main(int argc, char** argv){
	if(argc < 3){
		printf("Usage: %s no_of_chefs, no_of_deliverers\n", argv[0]);
	}
	int N = atoi(argv[1]);
	int M = atoi(argv[2]);
	pid_t pids[N+M];
	int semid = init_semaphore();   
    int shmid = init_shared_memory();
    struct shared_data *data = shmat(shmid, NULL, 0);

    for(int i = 0; i < MAX_PIZZE; i++){
    	data->oven[i] = -1;
    	data->table[i] = -1;
    }
    data->oven_counter = 0;
    data->table_counter = 0;

    printf("Creating workers\n");
	create_workers(N, CHEF_PATH, 0, pids);
	create_workers(M, DELIVERER_PATH, N, pids);

	signal(SIGINT, stop_program);
	
	while(running){
		usleep(300);
	}
	
	for(int i = 0; i < N+M; i++){
		kill(pids[i], SIGINT);
		wait(NULL);
	}
	shmdt(data); // detach the shared memory
	shmctl(shmid, IPC_RMID, 0);  // delete the shared memory from the system
	semctl(semid, -999, IPC_RMID);
	return 0;
}