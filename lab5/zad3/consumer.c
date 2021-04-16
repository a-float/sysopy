#include "shared.h"

#define MAX_SIZE 300

char **lines;
int* indices;

void write_to_line(FILE* fptr, int line_index, char* buffer){
    // printf("%d, %s\n", line_index, buffer);
    fseek(fptr, 0, SEEK_SET);
    char c;
    int lines = 0;
    char *rest = calloc(sizeof(char), MAX_SIZE);
    while((c = fgetc(fptr))!= EOF){
        // fprintf(stderr,"%c", c);
        if(c=='\n'){
            // fprintf(stderr,"passed a newline\n");
            lines++;
            if(lines == line_index+1){
                fseek(fptr, -1, SEEK_CUR);
                // fprintf(stderr,"breaking out\n");
                int read = fread(rest, sizeof(char), MAX_SIZE-1, fptr);
                fseek(fptr, -read, SEEK_CUR);
                // printf("the rest is '%s'\n", rest+1);
                break;
            }
        }
    }
    if(lines != line_index){
        for(int i = 0; i < line_index - lines; i++){
            fprintf(fptr, "\n");
        }
    }
    fprintf(fptr, "%s\n%s", buffer,rest+1);
    free(rest);
}

int main(int argc, char** argv) {
    if(argc < 4){
        printf("Usage: %s fifo_pipe out_file buff_size\n", argv[0]);
        return 1;
    }
    FILE* pipe = open_file(argv[1], "r");
    FILE* out_fp = open_file(argv[2], "r+w");
    int read, buff_size = atoi(argv[3])+3;  //for :i:
    char* buffer = calloc(buff_size, sizeof(char));
    

    while((read = fread(buffer, sizeof(char), buff_size, pipe)) > 0) {
        buffer[2] = 0;
        write_to_line(out_fp, atoi(buffer+1)-1, buffer+3);
    }
    printf("Consumer is done\n");
    fclose(out_fp);
    fclose(pipe);
    return 0;
}