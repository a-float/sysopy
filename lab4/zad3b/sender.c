#define SENDER
#include "shared.h"

int main(int argc, char** argv) {
	if(argc < 4){
		printf("Usage: %s catcher_pid signal_count send_mode\n", argv[0]);
		exit(1);
	}
	if(argc > 4){
		printf("Multiple catchers. Clear the processes.\n");
		exit(1);
	}
	bool is_kill = strcmp(argv[3], "KILL")==0 ? true : false;
	bool is_queue = strcmp(argv[3], "SIGQUEUE")==0 ? true : false;
	bool is_rt = strcmp(argv[3], "SIGRT") == 0 ? true : false;

	pid_t catcher_pid = atoi(argv[1]);
	int signal_count = atoi(argv[2]);

    printf("Sender: my pid is %d\n", getpid());
    printf("Sender: sending %d signals to pid %d \n", signal_count, catcher_pid);
	setup(is_rt);
	for(int i = 0; i < signal_count; i++){
		// printf("Sender: sending signal no %d\n", i);
		send_signal_to(catcher_pid, SIGNAL1);
		// printf("Sender: waiting for feedback no %d\n", i);
		sigsuspend(&wait_mask);
	}
	// printf("Sender: sending final signal\n");
	send_signal_to(catcher_pid, SIGNAL2);

	return 0;
}
