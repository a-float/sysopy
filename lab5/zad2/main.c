#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>


#define setup_info_sigaction(signal_name, block)\
  void handler_##signal_name(int sig, siginfo_t* info, void *context){\
    block\
    _exit(0);\
  }\
  act.sa_sigaction = handler_##signal_name;\
  act.sa_flags = SA_SIGINFO;\
  sigaction(signal_name, &act, NULL); //signal, new action, old action

#define setup_sigaction_handler(signal_name, flag, block)\
  void handler_##flag(int sig){\
    block\
  }\
  act.sa_handler = handler_##flag;\
  act.sa_flags = flag;\
  sigaction(signal_name, &act, NULL); //signal, new action, old action

#define check_out\
  if(out != NULL){\
    printf("%s", out);\
    out = NULL;\
  }


void siginfo_test(){
  struct sigaction act;
  pid_t forked;
  printf("\nSIGINFO flag:\n");
  setup_info_sigaction(SIGFPE, 
    printf("\tCause: %s\n\tSignal code: %d\n\tAddress at which fault occurred: %p\n", strsignal(info->si_signo), info->si_code, info->si_addr);
  );
  forked = fork();
  if(forked == 0){
    printf("\nTry to divide by zero:\n");
    int zero = 2 - 2;
    int a = 123/zero;
    a++;
    exit(0);
  }
  waitpid(forked, NULL, WUNTRACED);
  setup_info_sigaction(SIGSEGV, 
    printf("\tCause: %s\n\tSignal code: %d\n\tInvalid memory acces at: %p\n", strsignal(info->si_signo), info->si_code, info->si_addr);
  );
  forked = fork();
  if(forked == 0){
    printf("\nAccess bad memory address:\n");
    char* invalid_pointer = (char*) 777777;
    *invalid_pointer = 7;
    exit(0);
  }
  waitpid(forked, NULL, WUNTRACED);
  setup_info_sigaction(SIGILL, 
    printf("\tCause: %s\n\tSignal code: %d\n\tAddress at which fault occurred: %p\n", strsignal(info->si_signo), info->si_code, info->si_addr);
  );
  forked = fork();
  if(forked == 0){
    printf("\nOrder an illegal instructions:\n");
    asm("ud2");
    exit(0);
  }
  waitpid(forked, NULL, WUNTRACED);
}

void nocldstop_test(){
  struct sigaction act;
  pid_t forked;
  volatile char *out = NULL;
  printf("\nSA_NOCLDSTOP flag:\n\n");
  printf("Without the flag:\n");
  setup_sigaction_handler(SIGCHLD, 0,
    out = "\tparent: oh no, my child has stopped or died.\n";
  );
  forked = fork();
  if(forked == 0){
    printf("\tchild: stops itself\n");
    raise(SIGSTOP);
  }
  waitpid(forked, NULL, WUNTRACED);
  check_out
  forked = fork();
  if(forked == 0){
    printf("\tchild: exits\n");
    exit(0);
  }
  waitpid(forked, NULL, WUNTRACED);
  check_out

  printf("With the flag:\n");
  setup_sigaction_handler(SIGCHLD, SA_NOCLDSTOP,
    out = "\tparent: oh no, my child has stopped or died.\n";
  );
  forked = fork();
  if(forked == 0){
    printf("\tchild: stops itself\n");
    raise(SIGSTOP);
  }
  waitpid(forked, NULL, WUNTRACED);
  check_out
  forked = fork();
  if(forked == 0){
    printf("\tchild: exits\n");
    exit(0);
  }
  waitpid(forked, NULL, WUNTRACED);
  check_out
}


void resethand_test(){
  struct sigaction act;
  pid_t forked;
  volatile char *out = NULL;
  printf("\nSA_RESETHAND flag:\n\n");
  setup_sigaction_handler(SIGUSR1, SA_RESETHAND,
    out = "\tHello, I am handling the SIGUSR1 signal.\n";
  );
  forked = fork();
  if(forked == 0){
    printf("\tchild: raises SIGUSR1\n");
    while(1);
    exit(0);
  }
  printf("\tSending SIGUSR1 to the child for the first time\n");
  kill(forked, SIGUSR1);
  sleep(1);
  printf("\tSending SIGUSR1 to the child for the second time\n");
  kill(forked, SIGUSR1);
  waitpid(forked, NULL, WUNTRACED);
  check_out
}

int main() {
  resethand_test();
  siginfo_test();
  nocldstop_test();
  
  return 0;
}
