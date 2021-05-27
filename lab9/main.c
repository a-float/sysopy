#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define RED "\033[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\033[93m"
#define CYAN "\033[96m"
#define RESET_COL "\033[0m"

#define color_printf(color, content...) \
printf(color);                        \
printf(content);                      \
printf(RESET_COL);                    \
fflush(stdout);


void create_thread_with_arg(pthread_t *thread, void* (*func)(void *), int arg){
  int *cp = calloc(sizeof(int), 1);
  *cp = arg;
  pthread_create(thread, NULL, func, cp);
}

pthread_mutex_t santa_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elf_cond = PTHREAD_COND_INITIALIZER;

int waiting_reindeers = 0;
int waiting_elfs = 0;
int elf_sleep, reindeer_sleep;
int elf_array[3];
bool done = false;

void* santa(void* _) {
    // printf("Hello I am SANTA\n");
    while(true){
        while (waiting_reindeers != 9 && waiting_elfs != 3) {
            pthread_mutex_lock(&santa_mutex);
            color_printf(RED, "Mikolaj: zasypiam\n");
            pthread_cond_wait(&santa_cond, &santa_mutex);
            pthread_mutex_unlock(&santa_mutex);
        }
        if (waiting_reindeers == 9){
            pthread_mutex_lock(&reindeer_mutex);
            color_printf(RED, "Mikolaj: dostarczam zabawki\n");
            reindeer_sleep = rand()%2 + 2;
            pthread_cond_broadcast(&reindeer_cond);
            pthread_mutex_unlock(&reindeer_mutex);

            sleep(reindeer_sleep);
            color_printf(RED, "Mikolaj: zabawki dostarczone\n");
        }
        else if (waiting_elfs == 3){
            pthread_mutex_lock(&elf_mutex);
            color_printf(RED, "Mikolaj: rozwiazuje problemy elfow %d %d %d\n", elf_array[0], elf_array[1], elf_array[2]);
            elf_sleep = rand()%2+2;
            pthread_cond_broadcast(&elf_cond);
            pthread_mutex_unlock(&elf_mutex);
            sleep(elf_sleep);
        }
        if (done) return NULL;
    }
}

void* reindeer(void* arg) {
    int *id = (int*) arg;
    // printf("My reindeer id is %d\n", *id);
    while(true){
        // vacations
        sleep(rand()%5+1);
        pthread_mutex_lock(&reindeer_mutex);
        waiting_reindeers += 1;
        color_printf(YELLOW, "Renifer: czeka %d reniferow na Mikolaja, %d\n", waiting_reindeers, *id);
        if (waiting_reindeers == 9) {
            pthread_mutex_lock(&santa_mutex);
            color_printf(YELLOW, "Renifer: wybudzam Mikolaja, %d\n", *id);
            pthread_cond_broadcast(&santa_cond);
            pthread_mutex_unlock(&santa_mutex);

        }
        pthread_cond_wait(&reindeer_cond, &reindeer_mutex);
        
        waiting_reindeers -= 1;
        //deliver gifts
        pthread_mutex_unlock(&reindeer_mutex);
        sleep(reindeer_sleep);
        if (done) { free(id); return NULL; }
    }
}

void stand_in_queue(int elf_id){
    waiting_elfs += 1;
    color_printf(GREEN, "Elf: czeka %d elfow na Mikolaja, %d\n", waiting_elfs, elf_id);
    elf_array[waiting_elfs-1] = elf_id;
    if(waiting_elfs == 3){ // wakes up santa
        pthread_mutex_lock(&santa_mutex);
        color_printf(GREEN, "Elf: wybudzam Mikolaja, %d\n", elf_id);
        pthread_cond_broadcast(&santa_cond);
        pthread_mutex_unlock(&santa_mutex);
    }
}

void *elf(void* arg){
    int *id = (int*) arg;
    // printf("My Elf id is %d\n", *id);
    while(true){
        // work
        sleep(rand()%3+2);
        pthread_mutex_lock(&elf_mutex);
        if(waiting_elfs < 3){ // goes to santa to report the problem
            stand_in_queue(*id);    
        }
        else{ // waiting for a chance to raport the problem
            while(waiting_elfs >= 3){
                color_printf(GREEN, "Elf: czeka na powrot elfow, %d\n", *id);
                pthread_cond_wait(&elf_cond, &elf_mutex);
            }
            stand_in_queue(*id);    
        }
        pthread_cond_wait(&elf_cond, &elf_mutex); // waiting to be picked up by santa

        color_printf(GREEN, "Elf: Mikolaj rozwiazuje problem, %d\n", *id);
        elf_array[waiting_elfs-1] = 0;
        waiting_elfs -= 1;
        pthread_mutex_unlock(&elf_mutex);
        sleep(elf_sleep);
        if (done) { free(id); return NULL; }
    }}

int main(int argc, char** argv) {
    calloc(sizeof(int), 3);
    int ELFS = 10;
    int REINDEERS = 9;
    srand(time(NULL));

    pthread_t santa_thread;
    pthread_t *reindeer_threads = calloc(REINDEERS, sizeof(pthread_t));
    pthread_t *elf_threads = calloc(ELFS, sizeof(pthread_t));

    pthread_create(&santa_thread, NULL, santa, NULL);

    for(int i = 0; i < REINDEERS; i++) {
        create_thread_with_arg(&reindeer_threads[i], reindeer, i + 1);
    }
    for(int i = 0; i < ELFS; i++) {
        create_thread_with_arg(&elf_threads[i], elf, i + 1);
    }

    for(int i = 0; i < REINDEERS; i++) {
        pthread_join(reindeer_threads[i], NULL);
    }
    for(int i = 0; i < ELFS; i++) {
        pthread_join(reindeer_threads[i], NULL);
    }

    pthread_join(santa_thread, NULL);
    return 0;
}