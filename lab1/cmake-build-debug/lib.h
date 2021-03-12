#ifndef LAB1_MYLIBRARY2_H
#define LAB1_MYLIBRARY2_H
#include <stdio.h>

typedef struct Block{
    char** verses;
    int size;
} Block;

typedef struct Table{
    Block** blocks;
    int max_size;
} Table;

Block* merge_files(char *file1, char *file2);

Table* create_table(int size);

void delete_verse(Table* table, int block_index, int verse_index);
void delete_block(Table* table, int block_index);
void delete_table(Table** table);

#endif //LAB1_MYLIBRARY2_H
