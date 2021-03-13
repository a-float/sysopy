#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    Table* table = create_table(2);
    table->blocks[0] = merge_files("a.txt", "b.txt");
    printf("blocks: %d verses in block 0: %d\n",get_block_count(table), get_verse_count(table,0));

    print_table(table);

    Sequence* seq = create_sequence(2);
    add_file_pair(seq, "a.txt:b.txt");
    add_file_pair(seq, "b.txt:b.txt");
//    seq[0] = calloc(sizeof(char), 12);
//    seq[1] = calloc(sizeof(char), 12);
//    seq[0] = strcpy(seq[0],"a.txt:b.txt");
//    seq[1] = strcpy(seq[1],"a.txt:a.txt");

    merge_file_sequence(table, seq, 0);
//    delete_verse(table,0,3);
//    save_block_to_tmp_file(table, 0);
////    delete_block(table,1);
//    print_table(table);
//    printf("%d",load_block_from_tmp_file(table));
    print_table(table);
    delete_table(&table);

//    FILE* fp = fopen("a.txt", "rb");
//    printf("file size is %ld\n",get_file_size(fp));
//    fclose(fp);
}