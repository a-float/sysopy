#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>


#define setup_sigaction(signal_name, block)\
  void handler_##signal_name(int sig, siginfo_t* info, void *context){\
    block\
    _exit(0);\
  }\
  \
  /*handler if no SA_SIGINFO */ \
  act.sa_sigaction = handler_##signal_name;\
  act.sa_flags = SA_SIGINFO;\
  sigaction(signal_name, &act, NULL); //signal, new action, old action


int main() {
  struct sigaction act;
  setup_sigaction(SIGFPE, 
    printf("\tCause: %s\n\tSignal code: %d\n\tAddress at which fault occurred: %p\n", strsignal(info->si_signo), info->si_code, info->si_addr);
  );
  if(fork() == 0){
    printf("\nTry to divide by zero:\n");
    int zero = 2 - 2;
    int a = 123/zero;
    a++;
    exit(0);
  }
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  setup_sigaction(SIGSEGV, 
    printf("\tCause: %s\n\tSignal code: %d\n\tInvalid memory acces at: %p\n", strsignal(info->si_signo), info->si_code, info->si_addr);
  );
  if(fork() == 0){
    printf("\nAccess bad memory address:\n");
    char* invalid_pointer = (char*) 777777;
    *invalid_pointer = 7;
    exit(0);
  }
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  setup_sigaction(SIGILL, 
    printf("\tCause: %s\n\tSignal code: %d\n\tAddress at which fault occurred: %p\n", strsignal(info->si_signo), info->si_code, info->si_addr);
  );
  if(fork() == 0){
    printf("\nOrder an illegal instructions:\n");
    asm("ud2");
    exit(0);
  }
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  return 0;
}
