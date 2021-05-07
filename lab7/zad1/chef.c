#include "common.h"
#define get_to_oven(code...) 											\
	if(semop(semid, &p, 1) < 0){										\
        perror("semop p"); exit(13);									\
    }																	\
	code																\
	if(semop(semid, &v, 1) < 0){										\
        perror("semop v"); exit(13);									\
    }


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
	    perror("semget: ");
	}
	return id;
}

int get_pizza_count(int* place){
	int sum = 0;
	for(int i = 0; i < MAX_PIZZE; i++){
		if(place[i] != -1)sum++;
	}
	return sum;
}

int main(int argc, char** argv){
	if(argc < 2){
		printf("Usage: %s n\n", argv[0]);
		exit(0);
	}
	int chefid = atoi(argv[1]);
	srand(chefid*8);
	
	int semid = get_semaphore();
	int shmid = get_shared_memory();
	
    struct shared_data *data = shmat(shmid, NULL, 0);
    int *oven = data->oven;

    // TODO break condition, maybe signal?
    while(1){
    	int pizza_type = rand()%10;
    	int pizza_pos;
    	printf("(id:%d %s) Przygotowuje pizze: %d.\n", chefid, get_local_time(), pizza_type);
    	sleep(rand()%3+1);
    	get_to_oven(
    		pizza_pos = data->oven_counter++%MAX_PIZZE; // remember when I'll put the pizza
    		data->oven[pizza_pos] = pizza_type;  // put the pizza in
    		printf("(id:%d %s) Dodalem pizze: %d. Liczba pizz w piecu %d\n",
				chefid, get_local_time(), pizza_type, data->oven_counter);					
		)
        sleep(rand()%2+3);
        get_to_oven(
        	pizza_type = data->oven[pizza_pos]; // take the pizza out
    		data->oven[pizza_pos] = -1; // free a space in the oven
    		data->table_counter++%MAX_PIZZE; // advance the table counter
    		int table_pos = data->table_counter; 
    		data->table[table_pos] = pizza_type; // update the table
    		printf("(id:%d %s) Wyjmuję pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d.\n",
				chefid, get_local_time(), 
				pizza_type, 
				get_pizza_count(data->oven), 
				get_pizza_count(data->table));					
		)
    }

    shmdt(oven);
	return 0;
}