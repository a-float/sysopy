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
        sigsuspend(&mask);
    };

    printf("Catcher: received signals = %d\n", counter);
    printf("Catcher: sending %d signals back to pid %d\n", counter, sender_pid);
    //TODO could be moved to shared.h
    if(is_queue){
        union sigval val;
        val.sival_int = 0;
        for(int i = 0; i < counter; i++){
            sigqueue(sender_pid, SIGNAL1, val);
        }
        val.sival_int = counter;
        sigqueue(sender_pid, SIGNAL2, val);
    }
    else if(is_kill || is_rt){
        for(int i = 0; i < counter; i++){
            kill(sender_pid, SIGNAL1);
        }
        kill(sender_pid, SIGNAL2);
    }
    else{
        printf("Invalid send mode\n");
        exit(1);
    }
    printf("Catcher: finished resending signals\n");
    return 0;
}