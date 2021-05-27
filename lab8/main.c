#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#define MAX_LINE 75
#define TMP_NAME "tmp.pgm"
#define NUMBERS 0
#define BLOCKS 1

#define measure_real_time(code...) 		\
	struct timeval tv_start, tv_end;	\
	gettimeofday(&tv_start, NULL); 		\
	code; 								\
	gettimeofday(&tv_end, NULL); 		\
	printf("Total time is %lds %ldus\n", tv_end.tv_sec - tv_start.tv_sec, tv_end.tv_usec - tv_start.tv_usec);

typedef struct image{
	int width;
	int height;
	int max_val;
	int *data;
} image;

struct thread_arg{
	int variant;
	int no_of_threads;
	int thread_no;
	image *img;
	image *out_img;
};

void fprintf_img_data(FILE* where, image *img){
	for(int y = 0; y < img->height; y++){
		for(int x = 0; x < img->width; x++){
			fprintf(where, "%2d ", img->data[y*img->width+x]);
		}
		fprintf(where, "\n");
	}
}

int get_img_size(image *img){
	return img->width*img->height;
}

void remove_comments(const char *filename){
	FILE* fd = fopen(filename, "r");
	if(fd == NULL){
		perror("fopen fd: ");exit(11);
	}
	FILE* out = fopen(TMP_NAME, "w");
	if(out == NULL){
		perror("fopen out:");fclose(fd);exit(12);
	}
	size_t buffsize = MAX_LINE;
	char* buff = malloc(sizeof(char)*buffsize);
	int r, length;
	char *end;
	while((r = getline(&buff, &buffsize, fd)) > 0){
		char *res = strchr(buff, '#');
		end = "";
		if(res != NULL){
			length = (int)(res-buff);
			if(res != buff){
				end = "\n";
			}
		} else{
			length = MAX_LINE;
		}
		// printf("%.*s%s", length, buff, end);
		fprintf(out, "%.*s%s", length, buff, end);
		fflush(stdout);
	}
	// printf("\n");
	free(buff);
	fprintf(out, "\n");
	fclose(out);
	fclose(fd);
}

void load_data(const char *filename, image *img){
	remove_comments(filename);
	FILE *fd = fopen(TMP_NAME, "r");
	// int a,b,c;
	fscanf(fd, "%*s %d %d %d", &(img->width), &(img->height), &(img->max_val));
	// printf("%d %d %d", img->width, img->height, img->max_val);
	
	int img_size = img->width * img->height;
	// printf("\n");
	img->data = calloc(img_size, sizeof(int));
	for(int i = 0; i < img_size; i++){
		fscanf(fd, "%d", &(img->data[i]));
	}
	fclose(fd);
	#ifdef DEBUG 
	fprintf_img_data(stdout, img); 
	#endif
	if(remove(TMP_NAME) == -1){
	  perror("Could not delete the tmp file: ");
	}
}

void *thread_image_negative(void *vargp){
	struct timeval tv_start, tv_end;
	gettimeofday(&tv_start, NULL);
		int c = 0; // used to count the numver of changed values
	    struct thread_arg *arg = vargp;
	    // printf("Hi, my number is %d\n", arg->thread_no);

	    if(arg->variant == NUMBERS){
	    	int propor = (arg->img->max_val)/arg->no_of_threads+1;
	    	int min_num = propor*arg->thread_no;
	    	if(arg->thread_no == 0)min_num--; 
	    	int max_num = propor*(arg->thread_no+1);
	    	// printf("doin numbers from %d to %d\n", min_num, max_num);
	    	for(int i = 0; i < get_img_size(arg->img); i++){
	    		if(arg->img->data[i] >= min_num && arg->img->data[i] < max_num){
	    			arg->out_img->data[i] = arg->img->max_val-arg->img->data[i];
	    			c++;
	    		}
	    	}
	    }
	    else if(arg->variant == BLOCKS){
	    	int propor = arg->img->width/arg->no_of_threads + (arg->img->width%arg->no_of_threads != 0);
	    	// printf("%d\n", propor);
	    	int min_col = arg->thread_no*propor;
	    	int max_col = (arg->thread_no+1)*propor;
	    	if(arg->thread_no + 1 == arg->no_of_threads)max_col = arg->img->width;
	    	// printf("doin blocks from %d to %d\n", min_col, max_col);
	    	for(int x = min_col; x < max_col; x++){
	    		for(int y = 0; y < arg->img->height; y++){
	    			int index = y*arg->img->width + x;
	    			arg->out_img->data[index] = arg->img->max_val-arg->img->data[index];
	    			c++;
	    		}
	    	}
	    }
	    gettimeofday(&tv_end, NULL);
	    char* buff = calloc(60, sizeof(char));
		sprintf(buff, "Thread %d changed %d values in %lds %ldus", 
								arg->thread_no, 
								c,
								tv_end.tv_sec - tv_start.tv_sec,
								tv_end.tv_usec - tv_start.tv_usec);
		free(arg);
    return buff; // needs to be freed
}
   
int main(int argc, char **argv){
	//////////////////////////////////////////////INPUT
	if(argc < 5){
		printf("Usage: %s no_of_threads variant input_img out_img\n", argv[0]);
		exit(0);
	}
	int thread_count = atoi(argv[1]);
	printf("m = %d\n", thread_count);
	int variant;
	if(strcmp(argv[2], "blocks") == 0){
		variant = BLOCKS;
	} else if(strcmp(argv[2], "numbers") == 0){
		variant = NUMBERS;
	} else{
		fprintf(stderr, "Invalid variant name. Choose 'blocks' or 'numbers'\n");
		exit(1);
	}
	char* in_name = argv[3];
	char* out_name = argv[4];

	image img;
	load_data(in_name, &img);
	image out_img = img;
	out_img.data = calloc(get_img_size(&img), sizeof(int));
	struct thread_arg arg = {.variant=variant, 
							 .no_of_threads=thread_count,
							 .img = &img,
							 .out_img = &out_img
							};

	pthread_t thread_ids[thread_count];
	//////////////////////////////////////////////PARSING
	// printf("size is %ld\n", get_img_size(&img)*sizeof(int));
	measure_real_time(
		for(int i = 0; i < thread_count; i++){
			struct thread_arg *arg_cp = malloc(sizeof(struct thread_arg));
			memcpy(arg_cp, &arg, sizeof(struct thread_arg));
			arg_cp->thread_no = i;
			#ifdef DEBUG 
		    printf("Creating thread no %d\n", arg_cp->thread_no);
		    #endif
		    pthread_create(&(thread_ids[i]), NULL, thread_image_negative, arg_cp);
		}
		char *thread_out = NULL;
		for(int i = 0; i < thread_count; i++){
			pthread_join(thread_ids[i], (void**)&thread_out);
			if(thread_out != NULL){
				printf("%s\n", thread_out);
				free(thread_out);
			} else printf("out is a NULL\n");
			// printf("Joined the thread no %d\n", i);
		}
	)
	//////////////////////////////////////////////OUTPUT
	#ifdef DEBUG 
    printf("Showing the results:\n");
    fprintf_img_data(stdout, &out_img);
    #endif
    FILE* out = fopen(out_name, "w");
    if(out == NULL){
    	perror("fopen out_file: ");
    }
    else{
    	fprintf(out, "P2\n%d %d\n%d\n", out_img.width, 
    									out_img.height, 
    									out_img.max_val);
    	fprintf_img_data(out, &out_img); 
    	fclose(out);
    }
    free(img.data);
    free(out_img.data);
    exit(0);
}