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

int counter = 0;
bool listening = true;

#define setup(using_rt)\
	int SIGNAL1 = using_rt ? SIGRTMIN    : SIGUSR1;\
	int SIGNAL2 = using_rt ? SIGRTMIN+1  : SIGUSR2;\
	setup_mask\
	setup_actions


//normally accept no signals
//while sigsuspending receive signals 1 and 2
#define setup_mask\
	sigset_t mask;\
	sigfillset(&mask);\
	sigprocmask(SIG_SETMASK, &mask, NULL);\
	sigset_t wait_mask;\
	sigfillset(&wait_mask);\
	sigdelset(&wait_mask, SIGNAL1);\
	sigdelset(&wait_mask, SIGNAL2);\


pid_t sender_pid;
void signal_counter(int sig_no, siginfo_t* info, void* context){
 	// printf("%s: received SIGNAL 1\n", SPEAKER);
 	sender_pid = info->si_pid;
 	counter++;
}

void end_counting(int sig_no){
	// printf("%s: received SIGNAL 2\n", SPEAKER);
	listening = false;
}

#define setup_actions\
	struct sigaction act = {\
	  .sa_flags = SA_SIGINFO,\
	  .sa_sigaction = signal_counter\
	};\
	sigaction(SIGNAL1, &act, NULL);\
	struct sigaction act2 = {\
	  .sa_flags = SA_SIGINFO,\
	  .sa_handler = end_counting\
	};\
	sigaction(SIGNAL2, &act2, NULL);\


#define send_signal_to(pid, signal)\
	if(is_queue){\
		union sigval val;\
		val.sival_int = 0;\
		sigqueue(pid, signal, val);\
	}\
	else if(is_kill || is_rt){\
			kill(pid, signal);\
	}\
	else{\
        printf("Invalid send mode\n");\
        exit(1);\
    }
    