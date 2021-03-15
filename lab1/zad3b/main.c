#include "lib.h"
#include <sys/times.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef dynamic 
#include <dlfcn.h>
#endif

static struct tms tms_start, tms_end;
static clock_t clock_start, clock_end;

#define time_it(name, code_block)\
 do {\
     clock_start = times(&tms_start);\
     code_block\
     clock_end = times(&tms_end);\
     printf("%30s: ", name);\
     printf("real time: %ld ", clock_end - clock_start);\
     printf(" sys time: %ld ", tms_end.tms_stime - tms_start.tms_stime);\
     printf("user time: %ld\n", tms_end.tms_utime - tms_start.tms_utime);\
  } while (0)

int main(int argc, char **argv) {
  Table* table = NULL;

  #ifdef dynamic
    void* handle = dlopen("./libmerge.so", RTLD_NOW);
    if(!handle) {
      fprintf(stderr, "dlopen() %s\n", dlerror());
      exit(1);
    }
    Table* (*create_table)(int) = dlsym(handle, "create_table");
    void (*merge_file_sequence)(Table*, Sequence*, int) = dlsym(handle, "merge_file_sequence");
    Block* (*merge_files)(char*, char*) = dlsym(handle, "merge_files");
    void (*delete_block)(Table*, int) = dlsym(handle, "delete_block");
    //void (*delete_verse)(Table*, int, int) = dlsym(handle, "delete_verse");
    void (*delete_table)(Table**) = dlsym(handle, "delete_table");
    Sequence* (*create_sequence)(int) = dlsym(handle, "create_sequence");
    int (*delete_sequence)(Sequence**) = dlsym(handle, "delete_sequence");
    void (*add_file_pair)(Sequence*, char*) = dlsym(handle, "add_file_pair");
    void (*save_block_to_tmp_file)(Table*, int) = dlsym(handle, "save_block_to_tmp_file");

  #endif

   for (int i = 1; i < argc; i++) {
       char* arg = argv[i];

       if(strcmp(arg, "create_table") == 0) {
           if (table) free(table);
           int size = atoi(argv[++i]);
           table = create_table(size);
       }

       else if (strcmp(arg, "merge_files") == 0) {

          Sequence* seq = create_sequence(atoi(argv[++i]));
          char* file_pair = argv[++i];
          //adding the specified pair specified number of times to the seq
           for(int j = 0; j < seq->size; j++){
           		add_file_pair(seq, file_pair);
           }
           // timing(merge_file_sequence(table, seq, 0));
           time_it("merge_files",
                   merge_file_sequence(table, seq, 0);
           );
           delete_sequence(&seq);
       }
       else if (strcmp(arg, "save_to_file") == 0) {
           time_it("save_to_file", {
               for(int i = 0; i < table->size; i++){
                save_block_to_tmp_file(table, i);
               }
           });
       }
       else if (strcmp(arg, "add_remove") == 0) {
           int count = atoi(argv[++i]);
           char* file = argv[++i];
           // printf("%d %s\n",count, file);
           time_it("adding_and_removing", {
            for(int j = 0; j < count; j++){
               delete_block(table, j);
               table->blocks[j] = merge_files(file, file);
             }
           });
       }
       // else if (strcmp(arg, "remove_block") == 0) {
       //     int block_index = atoi(argv[++i]);
       //     time_it("remove_block", {
       //         delete_block(table, block_index);
       //     });
       // }

       // else if (strcmp(arg, "remove_row") == 0) {
       //     int block_index = atoi(argv[++i]);
       //     int operation_index = atoi(argv[++i]);
       //     time_it("remove_row", {
       //         delete_verse(table, block_index, operation_index);
       //     });
       // }
       else if (strcmp(arg, "remove_table") == 0) {
           time_it("remove_table", {
              delete_table(&table);
           });
           #ifdef dynamic
              dlclose(handle);
           #endif
           return 0;
       }
   }
   // print_table(table);
   delete_table(&table);
   #ifdef dynamic
      dlclose(handle);
   #endif
   return 0;
}