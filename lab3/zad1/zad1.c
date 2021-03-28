#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv){
	if(argc < 2){
		printf("Uzycie: %s [liczba procesow do wywolania]\n",argv[0]);
		return 0;
	}
	int n = atoi(argv[1]);

	for(int i = 0; i < n; i++){
		if(fork() == 0){
			printf("Komunikat z procesu nr %d\n",(int)getpid());
			exit(0);
		}
	}

	// oczekiwanie na zakonczenie pracy wszystkich 
	// procesow potomnych przed zakonczeniem procesu macierzystego
	int status = 0;
	// wait returns -1 when there is no process to wait for
	while(wait(&status) > 0);

	return 0;
}