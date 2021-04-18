#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
int main(){
	int fd[2];
	pipe(fd);
	if(fork() == 0){
		close(fd[0]);
		// char *buff = "echo 'im echoed'";
		dup2(fd[1],STDOUT_FILENO);
		// write(fd[1], buff, 14);
		execlp("echo", "echo", "asasas",  NULL);
		printf("Im still alive!\n");
		// exit(0);
	}
	else{
		close(fd[1]);
		char *line = malloc(sizeof(char)*100);
		int r = read(fd[0], line, 100);
		line[r-1] = 0;
		printf("I received '%s'\n", line);
		free(line);
	}
}