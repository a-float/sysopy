#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef sysIO
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

int main(int argc, char** argv){
	if(argc < 5){
		printf("Usage: %s file1 file2 n1 n2\n", argv[0]);
		return -1;
	}

#ifndef sysIO
	FILE *fp_in, *fp_out;

	if((fp_in = fopen(argv[1], "r")) == NULL){
		printf("Could not open file %s\n", argv[1]);
		return -1;
	}

	if((fp_out = fopen(argv[2], "w")) == NULL){
		printf("Could not open file %s\n", argv[2]);
		return -1;
	}
	
	char* from = argv[3];
	char* to = argv[4];
	// used to store letters of first string in case there is no full match 
	int c, index = 0, from_len = strlen(from);
	while((c = fgetc(fp_in)) != EOF){
		// printf("%c",c);
		if(c == from[index]){
			index++;
			if(index == from_len){ // doesnt count the null byte
				fprintf(fp_out, "%s", to);
				index = 0;
			}
		}
		else{
			if(c != from[index] && index > 0){
				fprintf(fp_out, "%.*s",index, from);
				index = 0;
			}
			fprintf(fp_out, "%c", c);
		}
	}
	fclose(fp_in);
	fclose(fp_out);

#else
	int f_in, f_out;

	if((f_in = open(argv[1], O_RDONLY)) == -1){
		printf("Could not open file %s\n", argv[1]);
		return -1;
	}

	if((f_out = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1){
		printf("Could not open file %s\n", argv[2]);
		return -1;
	}
	
	char* from = argv[3];
	char* to = argv[4];
	// used to store letters of first string in case there is no full match 
	int c, index = 0, from_len = strlen(from);
	while(read(f_in, &c, 1) != 0){
		// printf("%c",c);
		if(c == from[index]){
			index++;
			if(index == from_len){ // doesnt count the null byte
				write(f_out, to, strlen(to));
				index = 0;
			}
		}
		else{
			if(c != from[index] && index > 0){
				write(f_out, from, index*sizeof(char));
				index = 0;
			}
			write(f_out, &c, 1);
		}
	}
	close(f_in);
	close(f_out);

#endif
	return 0;
}