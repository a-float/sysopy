#define _POSIX_C_SOURCE 200809L
// #define __USE_GNU 200000L
#include <stdio.h>
#include <execinfo.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
/* get REG_EIP from ucontext.h */
#include <ucontext.h>

void bt_sighandler(int sig, siginfo_t *info,
				   void *secret) {

  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;
  ucontext_t *uc = (ucontext_t *)secret;

  /* Do something useful with siginfo_t */
  if (sig == SIGSEGV)
	printf("Got signal %d, faulty address is %p, "
		   "from %p\n", sig, info->si_addr, 
		   uc->uc_mcontext.gregs[REG_EIP]);
  // printf("Got signal %d, faulty address is %p, "
  //     "from ???\n", sig, info->si_addr);

  else
	printf("Got signal %d #92;n", sig);
	
  trace_size = backtrace(trace, 16);
  /* overwrite sigaction with caller's address */
  trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];

  messages = backtrace_symbols(trace, trace_size);
  /* skip first stack frame (points here) */
  printf("[bt] Execution path:#92;n");
  for (i=1; i<trace_size; ++i)
	printf("[bt] %s#92;n", messages[i]);

  exit(0);
}


int func_a(int a, char b) {

  char *p = (char *)0xdeadbeef;

  a = a + b;
  *p = 10;	/* CRASH here!! */

  return 2*a;
}


int func_b() {
  
  int res, a = 5;

  res = 5 + func_a(a, 't');

  return res;
}


int main() {

  /* Install our signal handler */
  struct sigaction sa;

  sa.sa_sigaction = (void *)bt_sighandler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_SIGINFO;

  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGTSTP, &sa, NULL);
  /* ... add any other signal here */

  #ifdef __USE_GNU
  printf("asdasdasdasd\n");
  #endif
  /* Do something */
  // printf("%d#92;n", func_b());
  return 0;
}