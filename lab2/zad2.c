//#include <stdio.h>
//#include <stdlib.h>
//
//int main(int argc, char** argv) {
//    FILE * fp;
//    char * line = NULL;
//    size_t len = 0;
//    ssize_t read;
//
//    printf("argc = %d %s \n",argc, argv[2]);
//    if (argc < 3)return -1;
//
//    char filter_char = argv[2][0];
//
//    fp = fopen(argv[1], "rb");
//    if(fp == NULL)return -1;
//
//    while((read = getline(&line, &len, fp)) != -1) {
////        printf("Retrieved line of length %zu: \n %s", read, line);
//        for(int i = 0; i < read; i++){
//            if(line[i] == filter_char) {
//                printf("%s", line);
//                break;
//            }
//        }
//    }
//    fclose(fp);
//    if(line) free(line);
//    return 0;
//}
