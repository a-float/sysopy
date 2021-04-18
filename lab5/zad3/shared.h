#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>

FILE* open_file(char* filename, char* flag){
	FILE* fp = fopen(filename, flag);
	if(fp == NULL){
		fprintf(stderr, "Could not open the file %s\n", filename);
		exit(1);
	}
	return fp;
}