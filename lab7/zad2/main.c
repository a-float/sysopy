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
 	int fd = shm_open(MEM_NAME, O_CREAT | O_RDWR, 0600);
	if (fd == -1){
		perror("create shm_open: ");
		exit(11);
	}
    return fd;
}

sem_t** init_semaphores(){
	unsigned int vals[] = {1, MAX_PIZZE, MAX_PIZZE, 0, 1}; // 1 space at the oven, full free oven, empty table, accesible table
	sem_t **sems = calloc(sizeof(sem_t),SEM_COUNT);
	for(int i = 0; i < SEM_COUNT; i++){
		sems[i] = sem_open(names[i], O_CREAT, 0666, vals[i]);
		if(sems[i] == SEM_FAILED){
			perror("get sem_open: ");
			exit(17);
		}
	}
	// int val;
	// for(int i = 0; i < SEM_COUNT; i++){
	// 	sem_getvalue(sems[i], &val);
	// 	printf("sem no %d is %d\n", i, val);
	// }
	return sems;
}

int main(int argc, char** argv){
	if(argc < 3){
		printf("Usage: %s no_of_chefs, no_of_deliverers\n", argv[0]);
	}
	int N = atoi(argv[1]);
	int M = atoi(argv[2]);
	pid_t pids[N+M];
	sem_t **sems = init_semaphores();   
    int fd = init_shared_memory();

	ftruncate(fd, sizeof(struct shared_data));
	if(fd == -1){
		perror("main ftruncate: ");
		exit(123);
	}
	struct shared_data *data = mmap(
		0, 
		sizeof(struct shared_data), 
		PROT_READ | PROT_WRITE, 
		MAP_SHARED, 
		fd, 
		0);

	if (data == MAP_FAILED){
		perror("main mmap: ");
		exit(30);
	}

	// starting values
    for(int i = 0; i < MAX_PIZZE; i++){
    	data->oven[i] = -1;
    	data->table[i] = -1;
    }
    data->oven_counter = 0;
    data->table_counter = 0;

    sleep(1);
    printf("Creating workers\n");
	create_workers(N, CHEF_PATH, 0, pids);
	create_workers(M, DELIVERER_PATH, N, pids);

	signal(SIGINT, stop_program);
	
	while(running){
		usleep(300);
	}
	
	for(int i = 0; i < N+M; i++){
		kill(pids[i], SIGINT);		
		pid_t pid = waitpid(WAIT_ANY, NULL, WNOHANG);
		// printf("QQXXXXXXXXXXXXXXXX pid is %d\n", pid);
		while(pid == 0){
			// printf("XXXXXXXXXXXXXXXX pid is %d\n", pid);
			kill(pids[i], SIGINT);		
			pid = waitpid(WAIT_ANY, NULL, WNOHANG);
			// sleep(1);
		}
		if(pid == -1){
			perror("waitpid: ");
			exit(111);
		}
	}
	munmap(data, sizeof(struct shared_data)); // detach the shared memory
	shm_unlink(MEM_NAME);  // delete the shared memory from the system
	// TODO remove the shared memory from the system?
	for(int i = 0; i < SEM_COUNT; i++){
		sem_close(sems[i]);
		sem_unlink(names[i]);
	}
	free(sems);
	// add sem_unlink
	return 0;
}