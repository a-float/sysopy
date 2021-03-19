#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    FILE * fp_in, *fp_out;
    char * line = calloc(sizeof(char), 53);// 50 + \n + \0
    ssize_t read;

    printf("argc = %d\n",argc);
    if (argc < 3)return -1;

    fp_in =  fopen(argv[1], "rb");
    fp_out = fopen(argv[2], "wb");
    if(fp_in == NULL || fp_out == NULL)return -1;

    int counter = 0;
    int c, size;
    while(fgets(line, 50, fp_in) != NULL){
        for(int i = 0; i < 50; i++){
            printf("%d %d '%c'\n",i, line[i], line[i]);
            if(line[i] == '\0')break;
        }
//
//        printf("%s",line);
//        fwrite(&line, sizeof(char), 50, fp_out);
//        if(line[49] != '\n')line[49]='\n';
//        printf("%d",strlen(line[0]));
//        fprintf(fp_out, "%s", line);
        fwrite(line, sizeof(line), 1, fp_out);
    }

    fclose(fp_in);
    fclose(fp_out);
    if(line) free(line);
    return 0;
}
