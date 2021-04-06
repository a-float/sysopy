#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <stdbool.h>

#ifdef SENDER
#define SPEAKER "Sender"
#endif

#ifdef CATCHER
#define SPEAKER "Catcher"
#endif

volatile int counter = 0;
volatile bool listening = true;

#define setup(using_rt)\
	int SIGNAL1 = using_rt ? SIGRTMIN    : SIGUSR1;\
	int SIGNAL2 = using_rt ? SIGRTMIN+1  : SIGUSR2;\
	setup_mask\
	setup_actions

void signal_counter(int sig_no){
 	// printf("%s: received SIGNAL 1\n", SPEAKER);
 	counter++;
}

pid_t sender_pid;
int catcher_received;
void end_counting(int sig_no, siginfo_t* info, void* context){
	// printf("%s: received SIGNAL 2\n", SPEAKER);
	sender_pid = info->si_pid;
	catcher_received = info->si_value.sival_int;
	listening = false;
}


#define setup_mask\
	sigset_t mask;\
	sigfillset(&mask);\
	sigdelset(&mask, SIGNAL1);\
	sigdelset(&mask, SIGNAL2);\
	sigprocmask(SIG_SETMASK, &mask, NULL);


#define setup_actions\
	signal(SIGNAL1, signal_counter);\
	struct sigaction act = {\
	  .sa_flags = SA_SIGINFO,\
	  .sa_sigaction = end_counting\
	};\
	sigaction(SIGNAL2, &act, NULL);
	