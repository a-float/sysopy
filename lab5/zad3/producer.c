#include "shared.h"
#include <time.h>

int main(int argc, char** argv) {
	if(argc < 5){
        printf("Usage: %s fifo_pipe row_no out_file buff_size\n", argv[0]);
        return 1;
    }
    // for(int i = 0; i < argc; i++){
    // 	printf("%s ", argv[i]);
    // }
    // printf("\nProducer %d\n", getpid());
    srand(time(0));
    int read, buff_size = atoi(argv[4]);

    FILE* pipe = open_file(argv[1], "r+");
    FILE* line_fp = open_file(argv[3], "r");
    
    char *buffer = calloc(sizeof(char), buff_size+1);

    while((read = fread(buffer, sizeof(char), buff_size, line_fp)) > 0) {
	    sleep(rand() % 1 + 1);
	    buffer[read] = 0;
	    // printf("Sending :%s:%s into the pipe\n", argv[2], buffer);
	    fprintf(pipe, ":%s:%s", argv[2], buffer);
	    fflush(pipe);
	}
	// printf("Producer %d is done\n", getpid());
	fclose(line_fp);
    // fclose(pipe);
	return 0;
}
