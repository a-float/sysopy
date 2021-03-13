//#include <sys/times.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <stdlib.h>
//#include <string.h>
//#include <time.h>
//#include "lib.h"
//
//static struct tms tms_start, tms_end;
//static clock_t clock_start, clock_end;
//
//#define timing(a) \
//    do { \
//        clock_t start = clock(); \
//        a; \
//        clock_t stop = clock(); \
//        printf("Elapsed: %f seconds\n", (double)(stop - start) / CLOCKS_PER_SEC); \
//    } while (0)
//
//#define time_it(name, code_block)\
//  do {\
//      clock_start = times(&tms_start);\
//      code_block;\
//      clock_end = times(&tms_end);\
//      printf("\n%s\n", name);\
//      printf("real time: %ld\n", clock_end - clock_start);\
//      printf(" sys time: %ld\n", tms_end.tms_stime - tms_start.tms_stime);\
//      printf("user time: %ld\n", tms_end.tms_utime - tms_start.tms_utime);\
//   } while (0)
//
//int main(int argc, char **argv) {
//
//    Table* table = NULL;
//
//    for (int i = 1; i < argc; i++) {
//        char* arg = argv[i];
//
//        if(strcmp(arg, "create_table") == 0) {
//            if (table) free(table);
//            int size = atoi(argv[++i]);
//            table = create_table(size);
//        }
//
//        else if (strcmp(arg, "merge_files") == 0) {
//            int sequence_length = atoi(argv[++i]);
//            char** sequence = calloc(sizeof(char*), sequence_length);
//
//            for(int j = 0; j < sequence_length; j++){
//                sequence[j] = calloc(sizeof(char), strlen(argv[i]));
//                strcpy(sequence[j], argv[++i]);
//            }
//            timing(merge_file_sequence(table, sequence, 0, sequence_length));
//            time_it("merge_files",
//                    merge_file_sequence(table, sequence, 0, sequence_length)
//            );
//            for(int j = 0 ; j < sequence_length; j++){
//                free(sequence[j]);
//            }
//            free(sequence);
//        }
//
//        else if (strcmp(arg, "remove_block") == 0) {
//            int block_index = atoi(argv[++i]);
//            time_it("remove_block", {
//                delete_block(table, block_index);
//            });
//        }
//
//        else if (strcmp(arg, "remove_row") == 0) {
//            int block_index = atoi(argv[++i]);
//            int operation_index = atoi(argv[++i]);
//            time_it("remove_row", {
//                delete_verse(table, block_index, operation_index);
//            });
//        }
//    }
//    // print_table(table);
//    delete_table(&table);
//    return 0;
//}