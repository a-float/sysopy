#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <sys/types.h>

#include <stdbool.h>
#include <unistd.h>

#define CHILD_OK SIGUSR1
#define PARENT_OK SIGUSR2
#define TESTER "tester.out"

bool parent_ok, child_ok;
#define CHILD_STATUS (child_ok ? "true" : "false")
#define PARENT_STATUS (parent_ok ? "true" : "false")

static void parent_ok_handler(int sig) { 
  parent_ok = true; 
  signal(PARENT_OK, parent_ok_handler);
}
static void child_ok_handler(int sig) { 
  // printf("received signal\n");
  child_ok = true; 
  signal(CHILD_OK, child_ok_handler);
}


void print_header(char* type){
  printf("│    │          %5s()         │       ignore       │       handler      │      pending       │        mask        │\n",type);
  printf("│ no │                   signal │  child  │  parent  │  child  │  parent  │  child  │  parent  │  child  │  parent  │\n");
}

char* itoa(int value) {
  static char buff[12];
  sprintf(buff, "%i", value);
  return strdup(buff);
}

#define make_test(tester, args...){\
  child_ok = parent_ok = false;\
  if (fork() == 0) execl(tester, tester, args, NULL);\
  /* should receive the signals here */ \
  waitpid(WAIT_ANY, NULL, WUNTRACED);\
  printf("  %5s  │   %5s  │", CHILD_STATUS, PARENT_STATUS);\
}

char* fork_commands[] = { "-i", "-h", "-p", "-m"};
char* exec_commands[] = { "-i", "-m", "-p"};

int main() {
  signal(PARENT_OK, parent_ok_handler);
  signal(CHILD_OK, child_ok_handler);
  
  printf("Start of fork test\n");
  print_header("fork");
  for(int s = 1; s <= 22; s++){
    printf("│ %2i │ %24s │", s, strsignal(s));
    for (int i = 0; i < 4; i++){
      make_test(TESTER, "-f", itoa(s), fork_commands[i]);
    }
    printf("\n");
  }
  printf("End of fork test\n");
  printf("Start of exec test\n");
  print_header("exec");
  for(int s = 1; s <= 22; s++){
    printf("│ %2i │ %24s │", s, strsignal(s));
    for (int i = 0; i < 3; i++){
      make_test(TESTER, "-e", itoa(s), exec_commands[i]);
    }
    printf("\n");
  }
  printf("End of exec test\n");
  return 0;
}