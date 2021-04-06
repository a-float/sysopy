#define SENDER
#include "shared.h"

int main(int argc, char** argv) {
	if(argc < 4){
		printf("Usage: %s catcher_pid signal_count send_mode\n", argv[0]);
		exit(1);
	}
	bool is_kill = strcmp(argv[3], "KILL")==0 ? true : false;
	bool is_queue = strcmp(argv[3], "SIGQUEUE")==0 ? true : false;
	bool is_rt = strcmp(argv[3], "SIGRT") == 0 ? true : false;

	if(argc > 4){
		printf("Multiple catchers. Clear the processes.\n");
		exit(1);
	}

	pid_t catcher_pid = atoi(argv[1]);
	int signal_count = atoi(argv[2]);
	setup(is_rt);
	printf("Sender: I'm going to send %d signals to catcher_pid %d.\n", signal_count, catcher_pid);

	if(is_queue){
		union sigval val;
		val.sival_int = 0;
		for(int i = 0; i < signal_count; i++){
			sigqueue(catcher_pid, SIGNAL1, val);
		}
		sigqueue(catcher_pid, SIGNAL2, val);
	}
	else if(is_kill || is_rt){
		for(int i = 0; i < signal_count; i++){
			kill(catcher_pid, SIGNAL1);
		}
		kill(catcher_pid, SIGNAL2);
	}
	else{
        printf("Invalid send mode\n");
        exit(1);
    }
	printf("Sender: finished sending signals\n");
	printf("Sender: listening to signals, my pid is %d\n", getpid());

	while(listening){
        sigsuspend(&mask);
	};

	if(is_queue)printf("Sender: received = %d while catcher got %d signals\n", counter, catcher_received);
	else printf("Sender: received signals = %d\n", counter);
	return 0;
}
