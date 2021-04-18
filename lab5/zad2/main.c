#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAX_BODY_SIZE 256

int main(int argc, char** argv) {
   char comm[MAX_BODY_SIZE];
    if(argc == 2){
    	char output[50];
		int read;
		printf("Listing the emails sorted by %s\n", argv[1]);
	  	if(strcmp(argv[1], "temat") == 0){
			sprintf(comm, "mail -H | sort -k 9");
			FILE* pfp = popen(comm, "r");
			while((read=fread(output, sizeof(char), sizeof(output), pfp)) >0){
				fwrite(output, sizeof(char), read, stdout);
			}
			pclose(pfp);
		}
		else if(strcmp(argv[1], "nadawca") == 0){
			sprintf(comm, "mail -H | sort -k 3");
			FILE* pfp = popen(comm, "r");
			while((read=fread(output, sizeof(char), sizeof(output), pfp)) > 0){
				fwrite(output, sizeof(char), read, stdout);
			}
			pclose(pfp);
		}
		else if(strcmp(argv[1], "data") == 0){
			sprintf(comm, "mail -H | sort -k4M -k5n -k6n");
			FILE* pfp = popen(comm, "r");
			while((read=fread(output, sizeof(char), sizeof(output), pfp)) > 0){
				fwrite(output, sizeof(char), read, stdout);
			}
			pclose(pfp);
		}
		else{
			printf("Invalid sort argument. Choose data or nadawca\n");
		}
    }
    else if(argc == 4){
	  	printf("Sending an email:\n");
	  	printf("To: %s\n", argv[1]);
	  	printf("Subject: %s\n", argv[2]);
	  	printf("Body: %s\n", argv[3]);
	  	
	  	int body_size = sprintf(comm, "mail -s '%s' '%s'", argv[2], argv[1]);
	  	FILE* pfp = popen(comm, "w");
	  	printf("Sending...\n");
	  	fwrite(argv[3], sizeof(char), body_size, pfp);
	  	pclose(pfp);
	  	printf("Done!\n");
    }
    else{
	  	printf("Usage: %s temat/nadawca/data\nor\n", argv[0]);
	  	printf("Usage: %s email subject body\n", argv[0]);
	  	exit(1);
    }
    return 0;
}
