#define CATCHER
#include "shared.h"

int main(int argc, char** argv) {
    if(argc < 2){
    printf("Usage: %s send_mode\n", argv[0]);
    exit(1);
    }
    bool is_kill = strcmp(argv[1], "KILL")==0 ? true : false;
    bool is_queue = strcmp(argv[1], "SIGQUEUE")==0 ? true : false;
    bool is_rt = strcmp(argv[1], "SIGRT") == 0 ? true : false;


    printf("Catcher: my pid is %d\n", getpid());
    setup(is_rt);
    
    while(listening){
        // printf("Catcher: waiting for signal no %d\n", counter);
        sigsuspend(&wait_mask);
        // printf("Catcher: confirming receival of signal no %d to pid %d\n", counter-1, sender_pid);
        send_signal_to(sender_pid, SIGNAL1);
        // printf("Catcher: sent the confirmation no %d\n", counter-1);
    };
    
    printf("Catcher: got %d signals\n",counter);
    return 0;
}