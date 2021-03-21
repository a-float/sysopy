#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LEN 256
#ifdef sysIO
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

int main(int argc, char** argv){//a
	char* file1 = calloc(sizeof(char),50);
	char* file2 = calloc(sizeof(char),50);
	if(argc == 1){
		printf("Input the first filename: ");
		scanf("%s", file1);
		printf("Input the second filename: ");
		scanf("%s", file2);
	}
	else if(argc == 3){
		strcpy(file1, argv[1]);
		strcpy(file2, argv[2]);
	}
	else{
		printf("Usage: %s or %s file1 file2", argv[0], argv[0]);
		return -1;
	}

#ifndef sysIO

	int opened_correctly = 1;
	FILE *fp1 = fopen(file1,"r");
	if(fp1 == NULL){
		printf("Could not open the file %s\n", file1);
		opened_correctly = 0;
	}
	FILE *fp2 = fopen(file2,"r");
	if(fp2 == NULL){
		printf("Could not open the file %s\n", file2);
		opened_correctly = 0;
	}
    free(file1);
    free(file2);
    if(opened_correctly == 0){
    	if(fp1)fclose(fp1);
    	if(fp2)fclose(fp2);
    	return -1;
	}

    //////////end of input

	int is_done = 0;
	char *line = calloc(sizeof(char), MAX_LEN);
	while(is_done == 0){
		is_done = 1;
		if(fgets(line, MAX_LEN, fp1) != NULL){
			is_done = 0;
			printf("%s", line);
		}
		if(fgets(line, MAX_LEN, fp2) != NULL){
			is_done = 0;
			printf("%s", line);
		}
	}
	free(line);
	fclose(fp1);
	fclose(fp2);

#else

	int opened_correctly = 1;
	int f1 = open(file1, O_RDONLY);
	if(f1 == -1){
		printf("Could not open the file %s\n", file1);
		opened_correctly = 0;
	}
	int f2 = open(file2, O_RDONLY);
	if(f2 == -1){
		printf("Could not open the file %s\n", file2);
		opened_correctly = 0;
	}
    free(file1);
    free(file2);
    if(opened_correctly == 0){
    	if(f1 != -1)close(f1);
    	if(f2 != -1)close(f2);
    	return -1;
	}

    //////////end of input

	int is_done = 0;
	char *line = calloc(sizeof(char), MAX_LEN);
	char c;
	while(is_done == 0){
		is_done = 1;
		while(read(f1, &c, 1) != 0){ //not end of file
			is_done = 0;
			printf("%c", c);
			if(c == '\n')break; //newline
		}
		while(read(f2, &c, 1) != 0){
			is_done = 0;
			printf("%c", c);
			if(c == '\n')break;
		}
	}
	free(line);
	close(f1);
	close(f2);

#endif

	return 0;
}