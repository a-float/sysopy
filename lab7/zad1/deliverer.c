#include "common.h"

int main(int argc, char** argv){
	if(argc < 2){
		printf("Usage: %s n\n", argv[0]);
		exit(0);
	}
	int workerid = atoi(argv[1]);
	srand(workerid*8);
	
	int semid = get_semaphore();
	int shmid = get_shared_memory();
	set_signal();
	
    struct shared_data *data = shmat(shmid, NULL, 0);
    int current_pizza;

    while(running){
    	change_sem(semid, TABLE_BUSY_SEM, -1); // release a spot on the table
		get_to_table(
			change_sem(semid, TABLE_FREE_SEM, 1); // for the chefs
			for(int i = 0; i < MAX_PIZZE; i++){
				if(data->table[i] != -1){	// found a pizza
					current_pizza = data->table[i];
					data->table[i] = -1; // mark as empty
					print_timestamp(workerid);
					printf("Pobieram pizze: %d. Liczba pizz na stole: %d\n",
						current_pizza,
						get_table_status(semid));
					break;
				}
			}
		);
		sleep(rand()%2+4); // drive to the client
		print_timestamp(workerid);
		printf("Dostarczam pizze: %d.\n",
						current_pizza);

		sleep(rand()%2+4); // drive back to the restaurant
    }

    shmdt(data);
	return 0;
}