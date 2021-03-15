#ifndef lib
#define lib
#include <stdio.h>

typedef struct Block{
    char** verses;
    int size;
} Block;

typedef struct Table{
    Block** blocks;
    int size;
} Table;

typedef struct Sequence{
    char** pairs;
    int size;
    int current_index;
} Sequence;


Table* create_table(int size);
Block* merge_files(char *file1, char *file2);
void merge_file_sequence(Table *table, Sequence* seq, int start); //start is where to start putting merge results in the table

int get_block_count(Table* table);
int get_verse_count(Table* table, int block_index);

Sequence* create_sequence(int size);
int add_file_pair(Sequence* seq, char* pair);

int load_block_from_tmp_file();
int save_block_to_tmp_file(Table* table, int block_index);

void delete_verse(Table* table, int block_index, int verse_index);
void delete_block(Table* table, int block_index);
void delete_table(Table** table);

void print_verse(char* verse);
void print_block(Table* table, int block_index);
void print_table(Table* table);

#endif
