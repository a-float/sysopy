#include <stdlib.h>
#include "mylibrary2.h"
#include <stdio.h>
#include <string.h>

typedef struct Node{
    char* value;
    struct Node* next;
} Node;

Table *create_table(int max_size) {
    Table* t = calloc(sizeof(Table), 1);
    t->blocks = calloc(sizeof(Block*),max_size);//TODO maybe table should hold Block* after all
    t->max_size = max_size;
    return t;
}

void push_back(Node *tail, char *new_verse) {
    Node* new_node = calloc(sizeof(Node), 1);
    new_node->value = new_verse;
    //new_node->value[strcspn(new_node->value, "\n")] = 0;  //removes the endline characters
    new_node->next = NULL;
    tail->next = new_node;
}

Node* parse_file_content(char* file, int* verse_count){
    FILE* fp = fopen(file,"rb");
    if(fp == NULL){
        fprintf(stderr, "Error while opening the file %s", file);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* content = calloc(sizeof(char),fsize+1);
    fread(content, 1, fsize, fp);
    fclose(fp);
    //no need to set the last bite because i used calloc

    Node* head = calloc(sizeof(Node), 1);
    head->next = NULL;
    Node* tail = head;

    char *start, *end, *buff;
    int verse_counter = 0;
    start = end = (char*) content;
    while( (end = strchr(start, '\n')) ){
        //printf("%d %.*s", count++, (int)(end - start + 1), start);
        buff = calloc(sizeof(char), end - start + 1 + 1);   //TODO these +1+1 may be wrong
        strncpy(buff, start, end-start+1);
        buff[end-start+1] = 0;
//        printf("%d %s\n",verse_counter, buff);
        push_back(tail, buff); //don't clear the buffer, pointer to it is kept in the node list
//        printf("getting verse %d %d %d\n",start ,end, fsize);
        tail = tail->next;
        start = end + 1;
        verse_counter++;
    }
    //getting the last verse without the \n character
    end = &content[fsize-1]; //&content[fsize-1] is the last character (not \0)
//    printf("the last char? %c%c%c%c\n", content[fsize-3],content[fsize-2],content[fsize-1],content[fsize]);
    if(start != end){ //the last verse did not end with \n
//        printf("there is one more %c %c %d %d\n",*start ,*end, fsize, &content[fsize-1]);
        buff = calloc(sizeof(char), end-start+1+1); //TODO fix the +-1
        strncpy(buff, start, end-start+1);
        push_back(tail, buff); //don't clear the buffer, pointer to it is kept in the node list
        //no need to update tails as its never used again
        verse_counter++;
    }
    *verse_count = verse_counter;
    free(content);
    return head;
}

Block *merge_files(char* file1, char* file2){
    int verse_count1, verse_count2;
    Node* heads[2];
    heads[0] = parse_file_content(file1, &verse_count1);
    heads[1] = parse_file_content(file2, &verse_count2);
    if(heads[0] == NULL || heads[1] == NULL){
        return NULL;
    }
    else{
        heads[0] = heads[0]->next;  //TODO maybe remove the guardian to begin with
        heads[1] = heads[1]->next;
    }
    int verse_count = verse_count1 + verse_count2;

    Block* block = calloc(sizeof(Block), 1);
    block->size = verse_count;
    block->verses = calloc(sizeof(char*), verse_count);
    Node* tmp;

    int verse_index;
    for(int i = 0; i < 2; i++){
        verse_index = i;
        while(heads[i] != NULL){
            block->verses[verse_index] = heads[i]->value;
            tmp = heads[i];
            heads[i] = heads[i]->next;
            free(tmp); //not freeing the verse buffers, just copying the pointers
            verse_index+=2;
        }
    }
    return block;
}

int merge_file_sequence(Table* table, char** sequence){
    char* files[2];
    for(int i = 0; i < table->max_size; i++) {
        if(table->blocks[i]) delete_block(table, i);
        files[0] = strtok(sequence[i], ":");
        files[1] = strtok(NULL, "");
        table->blocks[i] = merge_files(files[0], files[1]);
    }
}

void delete_verse(Table *table, int block_index, int verse_index) { //TODO should it be void?
    Block* block = table->blocks[block_index];
    if(block->verses[verse_index] != NULL) {
        free(block->verses[verse_index]);
        block->verses[verse_index] = NULL;
    }
}
void delete_block(Table *table, int block_index) { //TODO should it be void?
    Block* block = table->blocks[block_index];
    if(block != NULL) {
        for(int i = 0; i < block->size; i++) {
            delete_verse(table, block_index, i);
        }
        free(table->blocks[block_index]);
        table->blocks[block_index] = NULL;
    }
}
void delete_table(Table** table){
    for(int i = 0; i < (*table)->max_size; i++){
        delete_block(*table, i);
    }
    free((*table)->blocks);
    free(*table);
    *table = NULL;
}



//int open_files(char *file1, char *file2, FILE **fp1, FILE **fp2) {  //TODO make it separate open file functions?
//    *fp1 = fopen(file1, "rb");
//    if(fp1 == NULL){
//        fprintf(stderr, "Error while opening the file %s", file1);
//        return -1;
//    }
//    *fp2 = fopen(file2, "rb");
//    if(fp2 == NULL){
//        fprintf(stderr, "Error while opening the file %s", file2);
//        return -1;
//    }
//    return 1;
//}
