#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_CLIENT_CREATION_OFFSET 3
#define MAX_BARBER_ACTION_TIME 5
#define MAX_CLIENT_RETRY_DELAY 2
#define SLEEPING 0
#define GIFTING 1
#define HELPING 2

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

pthread_mutex_t reindeer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeer_cond = PTHREAD_COND_INITIALIZER;

int waiting_reindeers = 0, waiting_elfs = 0;
int santa_state = SLEEPING;
bool done = false;

void* santa(void* _) {
    printf("Hello I am SANTA\n");
    while(true){
        pthread_mutex_lock(&reindeer_mutex);
        while (waiting_reindeers != 9) {
            color_printf(RED, "Mikolaj: Zasypiam\n");
            santa_state = SLEEPING;
            pthread_cond_wait(&santa_cond, &reindeer_mutex);
        }
        color_printf(RED, "Mikolaj: Dostarczam zabawki\n");
        waiting_reindeers = 0;
        pthread_cond_broadcast(&reindeer_cond);
        pthread_mutex_unlock(&reindeer_mutex);
        santa_state = GIFTING;
        
        sleep(rand()%2+2);
        if (done) return NULL;
    }
}

void* reindeer(void* arg) {
    int *id = (int*) arg;
    printf("My reindeer id is %d\n", *id);
    while(true){
        sleep(rand()%5+1);
        pthread_mutex_lock(&reindeer_mutex);
        waiting_reindeers += 1;
        color_printf(YELLOW, "Renifer: czeka %d reniferow na Mikolaja, %d\n", waiting_reindeers, *id);
        if (waiting_reindeers == 9 && santa_state == SLEEPING) {
          color_printf(YELLOW, "Renifer: wybudzam Mikolaja, %d\n", *id);
          pthread_cond_broadcast(&santa_cond);
        }
        pthread_cond_wait(&reindeer_cond, &reindeer_mutex);
        pthread_mutex_unlock(&reindeer_mutex);

        sleep(rand()%2+2);

        if (done) { free(id); return NULL; }
    }
}

int main(int argc, char** argv) {
    // int ELFS = 10;
    int REINDEERS = 10;
    srand(time(NULL));

    pthread_t santa_thread;
    pthread_t *reindeer_threads = calloc(REINDEERS,sizeof(pthread_t));
    pthread_create(&santa_thread, NULL, santa, NULL);

    for(int i = 0; i < REINDEERS; i++) {
        create_thread_with_arg(&reindeer_threads[i], reindeer, i + 1);
    }

    for(int i = 0; i < REINDEERS; i++) {
        pthread_join(reindeer_threads[i], NULL);
    }
    pthread_join(santa_thread, NULL);
    return 0;
}