#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LEN 50
#ifdef sysIO
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

int main(int argc, char** argv) {
#ifndef sysIO
    FILE * fp_in, *fp_out;
    char * line = calloc(sizeof(char), MAX_LEN + 2);// 50 + \n + \0
    
    if (argc < 3){
        printf("Usage: %s in_file and out_file.\n", argv[0]);
        return -1;
    }

    fp_in =  fopen(argv[1], "r");
    if(fp_in == NULL){
        printf("Could not open file %s\n", argv[1]);
        return -1;
    }
    fp_out = fopen(argv[2], "w");
    if(fp_out == NULL){
        printf("Could not open file %s\n", argv[2]);
        return -1;
    }

    int i=0;
    while(fgets(line, MAX_LEN+1, fp_in) != NULL){
        i = 0;
        while(i<MAX_LEN+1){  //loop over read characters
            if(i == MAX_LEN){ //if the line is longer than 50 character (no null at the end)
                if(line[i-1]!=10){  //if the line does not end with a newline
                    line[i] = 10;  //overwrites the null to the newline
                    line[i+1] = 0; //add a new null
                    i++;    //string size has increased by one
                }
                break;
            }
            if(line[i] == '\0')break;
            i++;
        }
        // printf("showing string:\n");
        // print_str(line);
        fprintf(fp_out, "%s", line);
    }

    fclose(fp_in);
    fclose(fp_out);
    if(line) free(line);

#else
    int f_in =  open(argv[1], O_RDONLY);
    if(f_in == -1)return -1;
    int f_out = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if(f_out == -1)return -1;

    int line_index=0;
    char c, *line = calloc(sizeof(char), MAX_LEN+2);
    while(read(f_in, &c, 1) != 0){ //not eof
        strcpy(&line[line_index], &c);
        line_index++;
        if(line[line_index-1] == '\n'){
            write(f_out, line, line_index-1);
            line_index = 0;
        }
        else if(line_index == MAX_LEN){ //need for break
            line[line_index] = '\n';
            write(f_out, line, line_index+1);
            line_index = 0;
        }
    }
    write(f_out, line, line_index);

    close(f_in);
    close(f_out);
    free(line);


#endif
    return 0;
}
