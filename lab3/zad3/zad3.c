#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>

int main(int argc, char** argv){
	
	DIR* start_dir;
	// int max_depth = 99999;

	///////////////// input

	if(argc < 2){
		printf("Usage: %s start_dir (max_depth)\n",argv[0]);
		return 0;
	}
	else if(argc < 3){
		start_dir = opendir(argv[1]);
		if(start_dir == NULL){
			printf("Could not open the staring directory.\n");
			return -1;
		}
	}
	// else if(argc < 4){
		// max_depth = atoi(argv[2]);
	// }

	///////////////// end of input

	printf("%s %p\n", argv[1], start_dir);

	struct dirent *dir;
	while((dir = readdir(start_dir)) != NULL){
		// printf("Bong\n");q
		printf("%s\n",dir->d_name);
	}





	closedir(start_dir);

	// int status = 0;
	// while(wait(&status) > 0);

	return 0;
}