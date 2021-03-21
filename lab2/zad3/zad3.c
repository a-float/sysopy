#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// #define DATE_FILE "./zad3/dane.txt"
#define MAX_LEN 40
#define OUT_EVEN "a.txt"
#define OUT_7OR0 "b.txt"
#define OUT_SQUARES "c.txt"

#ifdef sysIO
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#endif

#ifndef sysIO
FILE* my_fopen(char* file_name, char* flag){
	FILE *fp = fopen(file_name, flag);
	if(fp == NULL){
		printf("Could not open the file %s\n", file_name);
		return NULL;
	}	
	return fp;
}
#else
int my_open(char* file_name, int flag, int mode){
	int f = open(file_name, flag, mode);
	if(f == -1){
		printf("Could not open the file %s\n", file_name);
		return -1;
	}	
	return f;
}
#endif

int main(int argc, char** argv){

	if(argc<2){
		printf("Usage: %s input_filename\n",argv[0]);
		return -1;
	}

#ifndef sysIO
	FILE *fp_in = my_fopen(argv[1],"r");
	if(fp_in == NULL)return -1;

	FILE *fp_even = my_fopen(OUT_EVEN,"w");
	if(fp_even == NULL)return -1;

	FILE *fp_7or0 = my_fopen(OUT_7OR0,"w");
	if(fp_7or0 == NULL)return -1;

	FILE *fp_squares = my_fopen(OUT_SQUARES,"w");
	if(fp_squares == NULL)return -1;

	char* line = calloc(sizeof(char), MAX_LEN);
	int num, even_counter = 0;
	while(fgets(line, MAX_LEN, fp_in) != NULL){
		num = atoi(line);
		if(num%2 == 0)even_counter++;
		if(num/10%10 == 7 || num/10%10 == 0){
			fprintf(fp_7or0, "%d\n", num);
		}
		double root = sqrt(num);
		if(ceil(root) == floor(root)){
			fprintf(fp_squares, "%d\n", num);
		}
		// printf("%d %d\n",num ,num/10%10);
	}
	fprintf(fp_even, "Liczb parzystych jest %d\n", even_counter);

	if(line) free(line);
	fclose(fp_even);
	fclose(fp_squares);
	fclose(fp_7or0);
	fclose(fp_in);

#else

	int f_in = my_open(argv[1], O_RDONLY, 0666);
	if(f_in == -1)return -1;

	int f_even = my_open(OUT_EVEN, O_WRONLY | O_TRUNC | O_CREAT, 0666);
	if(f_even == -1)return -1;

	int f_7or0 = my_open(OUT_7OR0, O_WRONLY | O_TRUNC | O_CREAT, 0666);
	if(f_7or0 == -1)return -1;

	int f_squares = my_open(OUT_SQUARES, O_WRONLY | O_TRUNC | O_CREAT, 0666);
	if(f_squares == -1)return -1;

	char c;
	char* line = calloc(sizeof(char), MAX_LEN);
	int num, line_index = 0, even_counter = 0;
	while(read(f_in, &c, 1) != 0){ //not end of file
		strcpy(&line[line_index], &c);
		line_index++;
		if(c == '\n'){
			num = atoi(line);
			// printf("%d\n",num);
			line[line_index] = '\0';
			if(num%2 == 0)even_counter++;
			if(num/10%10 == 7 || num/10%10 == 0){
				write(f_7or0, line, line_index);
			}
			double root = sqrt(num);
			if(ceil(root) == floor(root)){
				write(f_squares, line, line_index);
			}
			line_index = 0;
		}
	}
	char* buffer = calloc(50,sizeof(char));
	sprintf(buffer, "Liczb pierwszych jest %d", even_counter);
	int i = 0;
	while(buffer[i] != '\0')i++;
	write(f_even, buffer, i*sizeof(char));
	free(buffer);

	free(line);
	close(f_even);
	close(f_squares);
	close(f_7or0);
	close(f_in);

#endif
	return 0;
}