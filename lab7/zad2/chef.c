#include "common.h"

#define get_to_oven(code...) 					\
	sem_wait(sems[OVEN_DOOR_SEM]);				\
	code										\
	sem_post(sems[OVEN_DOOR_SEM]);

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

    while(running){
    	int pizza_type = rand()%10;
    	int pizza_pos;
    	sleep(rand()%3+1); // tworzenie pizzy
    	printf("(id:%d %s) Przygotowuje pizze: %d.\n", workerid, get_local_time(&(data->tv)), pizza_type);
    	
    	// change_sem(semid, OVEN_SEM, -1); // need place in the oven
    	sem_wait(sems[OVEN_SEM]);
    	get_to_oven(
    		pizza_pos = data->oven_counter++%MAX_PIZZE; // remember when I'll put the pizza
    		data->oven[pizza_pos] = pizza_type;  // put the pizza in
    		print_timestamp(workerid, &(data->tv));
    		printf("Dodalem pizze: %d. Liczba pizz w piecu: %d\n",
				pizza_type,
				get_oven_status(sems));	
			print_oven;
		)
        sleep(rand()%2+3);
        get_to_oven(
        	pizza_type = data->oven[pizza_pos]; // take the pizza out
    		data->oven[pizza_pos] = -1; // free a space in the oven (not needed)
    		print_timestamp(workerid, &(data->tv));
    		printf("Wyjmuję pizze: %d. Pizze w piecu: %d. Pizze na stole: %d.\n",
    			pizza_type,
				get_oven_status(sems)-1, // one is about to be taken out 
				get_table_status(sems));	// before this one is placed
    		print_oven;
    		// change_sem(semid, OVEN_SEM, 1); // release a place in the oven (needed)	
    		sem_post(sems[OVEN_SEM]);
		)
		
		// change_sem(semid, TABLE_FREE_SEM, -1); // one less spot at the table
		sem_wait(sems[TABLE_FREE_SEM]);
		get_to_table(
			printf("(id:%d %s) Klade pizze %d na stole. Liczba pizz na stole: %d.\n",
					workerid, get_local_time(&(data->tv)), 
					pizza_type, 
					get_table_status(sems)+1);	// before this one is placed
			int table_pos = data->table_counter++%MAX_PIZZE; // advance the table counter 
			data->table[table_pos] = pizza_type; // update the table	
			print_oven;
			// change_sem(semid, TABLE_BUSY_SEM, +1);	// for the deliverers
			sem_post(sems[TABLE_BUSY_SEM]);
		)
    }
    // printf("im a worker %d and i left the loop\n", getpid());
    for(int i = 0; i < SEM_COUNT; i++){
		sem_close(sems[i]);
	}
    free(sems);
    munmap(data, sizeof(struct shared_data)); // detach the shared memory

	return 0;
}