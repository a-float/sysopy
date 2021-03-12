#include "lib.h"
#include <stdio.h>

int main() {
    Table* table = create_table(2);
    table->blocks[0] = merge_files("a.txt", "b.txt");
    printf("blocks: %d verses in block 0: %d\n",get_block_count(table), get_verse_count(table,0));

    delete_verse(table,0,3);
    save_block_to_tmp_file(table, 0);
//    delete_block(table,1);
    print_table(table);
    printf("%d",load_block_from_tmp_file(table));
    print_table(table);
    delete_table(&table);

//    FILE* fp = fopen("a.txt", "rb");
//    printf("file size is %ld\n",get_file_size(fp));
//    fclose(fp);
}