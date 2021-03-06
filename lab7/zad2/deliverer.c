#include "common.h"

int main(int argc, char** argv){
	if(argc < 2){
		printf("Usage: %s n\n", argv[0]);
		exit(0);
	}
	int workerid = atoi(argv[1]);
	srand(workerid*8);
	
	sem_t **sems = get_semaphores();
	int fd = get_shared_memory();
	set_signal();
	
    struct shared_data *data = mmap(NULL, sizeof(struct shared_data),
     	PROT_WRITE, 
     	MAP_SHARED, 
     	fd, 
     	0);

    int current_pizza;

    while(running){
    	// change_sem(semid, TABLE_BUSY_SEM, -1); // release a spot on the table
    	sem_wait(sems[TABLE_BUSY_SEM]);
		get_to_table(
			// change_sem(semid, TABLE_FREE_SEM, 1); // for the chefs
			sem_post(sems[TABLE_FREE_SEM]);
			for(int i = 0; i < MAX_PIZZE; i++){
				if(data->table[i] != -1){	// found a pizza
					current_pizza = data->table[i];
					data->table[i] = -1; // mark as empty
					print_timestamp(workerid, &(data->tv));
					printf("Pobieram pizze: %d. Liczba pizz na stole: %d\n",
						current_pizza,
						get_table_status(sems));
					break;
				}
			}
		);
		sleep(rand()%2+4); // drive to the client
		print_timestamp(workerid, &(data->tv));
		printf("Dostarczam pizze: %d.\n",
						current_pizza);

		sleep(rand()%2+4); // drive back to the restaurant
    }
    for(int i = 0; i < SEM_COUNT; i++){
		sem_close(sems[i]);
	}
    munmap(data, sizeof(struct shared_data)); // detach the shared memory
    free(sems);
	return 0;
}