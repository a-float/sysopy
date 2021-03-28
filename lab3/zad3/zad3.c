#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>


int file_contains(const char* path, const char* target){
	FILE* fp = fopen(path, "r");
	if(fp == NULL){
		perror("Error: ");
		return -1;
	}
	int found = 0, to_find = strlen(target);
	char c;
	while((c = fgetc(fp)) != EOF){
		if(c == target[found]){
			found++;
			if(found == to_find)return 1;
		}
		else{
			found = 0;
		}
	}
	return 0;
}

int is_text_file(const char* path){
	char command[1024 + 100];
	snprintf(command, 100, "file %s | grep text >>/dev/null 2>>/dev/null", path);
	// printf("the command is %s\n", command);
	int status = system(command);
	// free(command); //TODO fix a lot of callocing
	if(status > 0)return 0;	//no match
	else return 1;	//something found
}

int is_dir(const char *name, const struct dirent *entry){
	if(entry->d_type == DT_UNKNOWN){	//filesystem does not support dir checking from the entry
		struct stat buf;
        stat(name, &buf);
        if(S_ISDIR(buf.st_mode)){
            return 1;
        }
        return 0;
	}
	else{
		if(entry->d_type == DT_DIR){
			return 1;
		}
		return 0;
	}
}

int search_dir(const char *name, const char *string_to_find, int depth, int max_depth){
	DIR *dir;
    struct dirent *entry;

    if((dir = opendir(name)) == NULL)return -1; // could not open the directory

    int indent = depth * 2;
    while ((entry = readdir(dir)) != NULL){
    	char path[1024];
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".git") == 0){
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        // printf("%d is visiting %s\n", (int)getpid(), path);

        if (is_dir(name, entry)){
            // printf("%*s[%s]\n", indent, "", entry->d_name);
            if(depth < max_depth){
            	if(fork() == 0){
            		search_dir(path, string_to_find, depth+1, max_depth);
            		exit(0);
            	}
            }
            
        }
        else {
            // printf("%*s- %s %d\n", indent, "", entry->d_name, is_text_file(path));
            if(is_text_file(path) && file_contains(path, string_to_find)){
            	printf("%*s- %d %s\n", indent, "", (int)getpid(), path);
            }
        }
    }
    closedir(dir);
    int status = 0;
	while(wait(&status) > 0);
	// printf("%d exits\n", (int)getpid());
	exit(0);
}


int main(int argc, char** argv){
	
	char *string_to_find;
	char *start_dir;
	int max_depth = 999999; //default value

	///////////////// input

	if(argc < 3){
		printf("Usage: %s string start_dir (max_depth)\n",argv[0]);
		return 0;
	}
	else if(argc > 3){
		string_to_find = argv[1];	
		start_dir = argv[2];
	}
	if(argc >= 4){
		max_depth = atoi(argv[3]);
	}

	///////////////// end of input

	if(fork() == 0){ //child
		search_dir(start_dir, string_to_find, 0, max_depth);
	}

	int status = 0;
	while(wait(&status) > 0);

	return 0;
}