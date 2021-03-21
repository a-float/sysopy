#define _GNU_SOURCE
#define MAX_LEN 256
#include <stdio.h>
#include <stdlib.h>
#ifdef sysIO
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#endif

int main(int argc, char** argv) {
#ifndef sysIO
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  size_t read;

  if (argc < 3){
    printf("Usage: %s znak plik\n",argv[0]);
    return -1;
  }

  char filter_char = argv[1][0];

  fp = fopen(argv[2], "r");
  if(fp == NULL){
    printf("Could not open the file %s\n", argv[2]);
    return -1;
  }

  while((read = getline(&line, &len, fp)) != -1) {
     // printf("Retrieved line of length %zu: \n %s", read, line);
     for(int i = 0; i < read; i++){
         if(line[i] == filter_char){
             printf("%s", line);
             break;
         }
     }
  }
  fclose(fp);
  if(line) free(line);

#else
  int f;

  if (argc < 3){
    printf("Usage: %s znak plik\n",argv[0]);
    return -1;
  }

  char filter_char = argv[1][0];

  f = open(argv[2], O_RDONLY);
  if(f == -1){
    printf("Could not open the file %s\n", argv[2]);
    return -1;
  }
  char c;
  char *line = calloc(MAX_LEN,sizeof(char));
  int line_index = 0;
  int has_filter_char = 0;
  while(read(f, &c, 1) != 0){ // not end of the file
    strcpy(&line[line_index], &c);
    line_index++;
    if(c == filter_char){
      has_filter_char = 1;
    }
    if(c == '\n'){
      line[line_index] = '\0';
      if(has_filter_char == 1)printf("%s", line);
      has_filter_char = 0; 
      line_index = 0;
    }
  }
  close(f);
  free(line);

#endif
  return 0;
}