#include <stdlib.h>
#include "lib.h"
#include <stdio.h>
#include <string.h>

#define TMP_FILENAME "block.tmp"

typedef struct Node {
    char *value;
    struct Node *next;
} Node;

Table *create_table(int size) {
    if(size <= 0){
        fprintf(stderr, "Cannot create a table of size smaller than 1");
        return NULL;
    }
    Table *t = calloc(sizeof(Table), 1);
    t->blocks = calloc(sizeof(Block *), size);  //TODO maybe table should hold Block* after all
    t->size = size;
    return t;
}

long get_file_size(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    return size;
}

void push_back(Node *tail, char *new_verse) {
    Node *new_node = calloc(sizeof(Node), 1);
    new_node->value = new_verse;
    //new_node->value[strcspn(new_node->value, "\n")] = 0;  //removes the endline characters
    new_node->next = NULL;
    tail->next = new_node;
}

int load_file(char **buffer, char *file, int *file_size) {
    FILE *fp = fopen(file, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error while opening the file %s\n", file);
        return -1;
    }
    long size = get_file_size(fp);
    *buffer = calloc(sizeof(char), size);
    fread(*buffer, sizeof(char), size, fp);
    fclose(fp);
    *file_size = size;
    return 0;
}

int save_block_to_tmp_file(Table *table, int block_index) {
    if (table == NULL) {
        fprintf(stderr, "The table doesnt exist");
        return -1;
    }
    Block *block = table->blocks[block_index];
    if (block == NULL) {
        fprintf(stderr, "The verse block doesnt exist");
        return -1;
    }
    FILE *fp = fopen(TMP_FILENAME, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Could not write open the tmp file");
        return -2;
    }
    for (int i = 0; i < block->size; i++) {   //TODO the empty verses are removed
        if (block->verses[i] != NULL) {
            fprintf(fp, "%s", block->verses[i]);
        }
    }
    fclose(fp);
    return 0;
}

Node *parse_file_content(char *file, int *verse_count) {
    char *content;
    int filesize;
    if (load_file(&content, file, &filesize) < 0) {
        fprintf(stderr, "Error while opening the file %s", file);
        return NULL;
    }
    Node *head = calloc(sizeof(Node), 1);
    head->next = NULL;
    Node *tail = head;

    char *start, *end, *buff;
    int verse_counter = 0;
    start = (char *) content;
    while ((end = strchr(start, '\n'))) {
        buff = calloc(sizeof(char), end - start + 1 + 1);   //TODO these +1+1 may be wrong
        strncpy(buff, start, end - start + 1);
        push_back(tail, buff); //don't clear the buffer, pointer to it is kept in the node list
        tail = tail->next;
        start = end + 1;
        verse_counter++;
    }
    //getting the last verse without the \n character
    end = &content[filesize - 1]; //&content[filesize-1] is the last character (not \0)
    if (start != &content[filesize]) { //the last verse did not end with \n
        buff = calloc(sizeof(char),
                      end - start + 1 + 1 + 1); //+1 to make up for end-start, +1 for \n, +1 for the null byte
        strncpy(buff, start, end - start + 1); //buff size - 1 for the null byte
        buff[end - start + 1] = '\n';
        push_back(tail, buff); //don't clear the buffer, pointer to it is kept in the node list
        //no need to update the tail as its never used again
        verse_counter++;
    }
    *verse_count = verse_counter;
    free(content);
    return head;
}

int load_block_from_tmp_file(Table *table) {
    if (table == NULL) {
        fprintf(stderr, "Cant load block into a non existent table");
        return -2;
    }
    int verse_count;
    Node *head = parse_file_content(TMP_FILENAME, &verse_count);

    if (head == NULL) {
        fprintf(stderr, "Error while loading block from the tmp file");
        return -1;
    }
    Block *block = calloc(sizeof(Block), 1);
    block->verses = calloc(sizeof(char *), verse_count);
    block->size = verse_count;

    Node *tmp = head;
    head = head->next;
    free(tmp);
    for (int i = 0; i < verse_count; i++) {
        block->verses[i] = head->value;
        tmp = head;
        head = head->next;
        free(tmp);
    }
    //inserting the new block in the first available table blocks element
    for (int i = 0; i < table->size; i++) {
        if (table->blocks[i] == NULL) {
            table->blocks[i] = block;
            return i;
        }
    }
    return -1;
}

Block *merge_files(char *file1, char *file2) {
    int verse_count1, verse_count2;
    Node *heads[2];
    heads[0] = parse_file_content(file1, &verse_count1);
    heads[1] = parse_file_content(file2, &verse_count2);
    if (heads[0] == NULL || heads[1] == NULL) {
        return NULL;
    } else {
        Node *tmp = heads[0];
        heads[0] = heads[0]->next;  //TODO maybe remove the guardian to begin with
        free(tmp);
        tmp = heads[1];
        heads[1] = heads[1]->next;
        free(tmp);
        //TODO remove the guardian, really.
    }
    int verse_count = verse_count1 + verse_count2;

    Block *block = calloc(sizeof(Block), 1);
    block->size = verse_count;
    block->verses = calloc(sizeof(char *), verse_count);
    Node *tmp;

    int verse_index = 0;
    while (verse_index < verse_count) {
        for (int i = 0; i < 2; i++) {
            if (heads[i] != NULL) {
                block->verses[verse_index] = heads[i]->value;
                tmp = heads[i];
                heads[i] = heads[i]->next;
                free(tmp);
                verse_index++;
            }
        }
    }
    return block;
}

void merge_file_sequence(Table *table, Sequence* seq, int start) {
    if(seq->size + start > table->size){
        fprintf(stderr, "Cannot merge files to a block outside the table");
        return;
    }
    char *files[2];
    for (int i = start; i < seq->size; i++) {
        if (table->blocks[i]) delete_block(table, i);
        files[0] = strtok(seq->pairs[i], ":");
        files[1] = strtok(NULL, ":");
        printf("Merging file %s and file %s\n",files[0], files[1]);
        table->blocks[i] = merge_files(files[0], files[1]);
    }
}

Sequence* create_sequence(int size){
    if(size <= 0){
        fprintf(stderr, "Cannot create a sequence of size smaller than 1");
        return NULL;
    }
    Sequence* seq = calloc(sizeof(Sequence),1);
    seq->size = size;
    seq->pairs = calloc(sizeof(char**), size);
    seq->current_index = 0;
    return seq;
}

int add_file_pair(struct Sequence* seq, char* pair){
    if(seq->current_index == seq->size){
        fprintf(stderr, "Cannot add a pair to the sequence. The sequence is full.");
        return -1;
    }
    seq->pairs[seq->current_index] = calloc(sizeof(char), strlen(pair));
    strcpy(seq->pairs[seq->current_index], pair);
    seq->current_index++;
    return 0;
}

void delete_verse(Table *table, int block_index, int verse_index) { //TODO should it be void?
    Block *block = table->blocks[block_index];
    if (block->verses[verse_index] != NULL) {
        free(block->verses[verse_index]);
        block->verses[verse_index] = NULL;
    }
}

void delete_block(Table *table, int block_index) { //TODO should it be void?
    Block *block = table->blocks[block_index];
    if (block != NULL) {
        for (int i = 0; i < block->size; i++) {
            delete_verse(table, block_index, i);
        }
        free(block->verses);
        free(table->blocks[block_index]);
        table->blocks[block_index] = NULL;
    }
}

void delete_table(Table **table) {
    for (int i = 0; i < (*table)->size; i++) {
        delete_block(*table, i);
    }
    free((*table)->blocks);
    free(*table);
    *table = NULL;
    remove(TMP_FILENAME);
}

int get_block_count(Table *table) {
    if (table == NULL || table->blocks == NULL) return -1;
    int counter = 0;
    for (int i = 0; i < table->size; i++) {
        if (table->blocks[i] != NULL)counter++;
    }
    return counter;
}

int get_verse_count(Table *table, int block_index) {
    if (table == NULL || table->blocks == NULL) return -1;
    Block *block = table->blocks[block_index];
    if (block == NULL) {
        return -1;
    }
    int counter = 0;
    for (int i = 0; i < block->size; i++) {
        if (block->verses[i] != NULL)counter++;
    }
    return counter;
}

void print_verse(char *verse) {
    if (&verse[0] == NULL) printf("[deleted]\n");
    else printf("%s", verse);
}

void print_block(Table *table, int block_idx) {
    Block *block = table->blocks[block_idx];
    if (block == NULL)return;
    for (int i = 0; i < block->size; i++) {
        printf("%d. ", i);
        print_verse(block->verses[i]);
    }
}

void print_table(Table *table) {
    if (table == NULL) {
        printf("The table is NULL");
        return;
    }
    for (int i = 0; i < table->size; i++) {
        if (table->blocks[i] != NULL) {
            printf("\nBlock no %d (size = %d):\n", i, table->blocks[i]->size);
            print_block(table, i);
        } else {
            printf("\nBlock no %d is NULL\n", i);
        }
    }
}