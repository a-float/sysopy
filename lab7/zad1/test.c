#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h> 
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <stdbool.h>
#include <signal.h>

time_t start_secs = -1;
char *get_local_time(){
	char* buff = malloc(sizeof(char)*30);
	struct timeval tv;
	gettimeofday(&tv,NULL);
	if(start_secs == -1)start_secs = tv.tv_sec;
	// tv.tv_sec // seconds
	// tv.tv_usec // microseconds
	sprintf(buff, "%lds%2.ldms", tv.tv_sec-start_secs, tv.tv_usec);
	return buff;
}

int main(){
	for(int i = 0; i < 100; i ++){
		char* t = get_local_time();
		printf("%s\n", t);
		free(t);
	}
	return 0;
}