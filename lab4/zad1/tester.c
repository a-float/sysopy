#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>
#define EXE "./exec.out"
#define CHILD_OK SIGUSR1
#define PARENT_OK SIGUSR2


bool use_exec = false;
static pid_t ppid;
char* itoa(int value) {
  static char buff[12];
  sprintf(buff, "%i", value);
  return strdup(buff);
}

void ignore_test(int sig) {
  signal(sig, SIG_IGN);
  if (fork() == 0) {
    if (use_exec) execl(EXE, EXE, "-i", itoa(sig), itoa(ppid), NULL);
    raise(sig);
    kill(ppid, CHILD_OK);
  }
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  raise(sig);
  kill(ppid, PARENT_OK);
}

void parent_handler(int _) { kill(ppid, PARENT_OK); }
void child_handler(int _) { kill(ppid, CHILD_OK); }
void handler_test(int sig) {
  signal(sig, child_handler);
  if (fork() == 0) raise(sig);
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  signal(sig, parent_handler);
  raise(sig);
}

void exit_handler(int _) { _exit(0); }
void mask_test(int sig) {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, sig);
  sigprocmask(SIG_SETMASK, &mask, NULL);
  signal(sig, exit_handler); //if the signal arrived, mask did not work.
  if (fork() == 0) {
    if (use_exec) execl(EXE, EXE, "-m", itoa(sig), itoa(ppid), NULL);
    raise(sig);
    kill(ppid, CHILD_OK);
  }
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  raise(sig);
  kill(ppid, PARENT_OK);
}

void pending_test(int sig) {
  sigset_t newmask;
  sigemptyset(&newmask);
  sigaddset(&newmask, sig);
  sigprocmask(SIG_SETMASK, &newmask, NULL);

  raise(sig);
  sigset_t mask;
  if (fork() == 0) {
    raise(sig);
    if (use_exec) execl(EXE, EXE, "-p", itoa(sig), itoa(ppid), NULL);
    sigpending(&mask);
    if (sigismember(&mask, sig)) 
      kill(ppid, CHILD_OK);
  }
  waitpid(WAIT_ANY, NULL, WUNTRACED);
  sigpending(&mask);
  if (sigismember(&mask, sig)) kill(ppid, PARENT_OK);
}


int main(int argc, char** argv) {
  ppid = getppid();
  if (strcmp(argv[1], "-f")) use_exec = false;
  if (strcmp(argv[1], "-e")) use_exec = true;
  int sig = atoi(argv[2]);

  if(strcmp(argv[3],"-i")){
    ignore_test(sig);
  }
  else if(strcmp(argv[3],"-h")){
    handler_test(sig);
  }
  else if(strcmp(argv[3],"-m")){
    mask_test(sig);
  }
  else if(strcmp(argv[3],"-p")){
    pending_test(sig);
  }
}