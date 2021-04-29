#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <mqueue.h>

#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 100
#define MAX_NAME_SIZE 30

#define SERVER_KEY_PATHNAME (getpwuid(getuid())->pw_dir)
#define PROJECT_ID 'x'
#define QUEUE_PERMISSIONS 0620

#define RED "\033[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\033[93m"
#define CYAN "\033[96m"
#define RESET_COL "\033[0m"

typedef int qid_t;

#define tick usleep(200 * 1000)

#define color_printf(color, content...) \
printf(color); \
printf(content); \
printf(RESET_COL); \
fflush(stdout);

typedef enum msg_type { // the bigger the number the bigger the priority
	Message = 2L,
	Error,
	Connect,
	Init,
	List,
	Disconnect,
	Stop
} msg_type;

struct message {
	long mtype;
	union content {
		union request {
			char Init[MAX_NAME_SIZE];
	 		struct {id_t cid;} Stop, List, Disconnect;
			struct {id_t cid; id_t peer_id;} Connect;
		} request;

		union response {
			struct {id_t cid;} Init, Stop, Disconnect;
			struct {char peer_name[MAX_NAME_SIZE];} Connect;
			char Message[MAX_MSG_SIZE];
		} response;
	} content;
};

char* get_server_name(){
	key_t key = ftok(SERVER_KEY_PATHNAME, PROJECT_ID);
	char *name = malloc(sizeof(char)*11);
	name[0] = '/';
	snprintf(&name[1], 10, "%d", key);
	printf("server name is %s\n", name);
	return name;
}

int create_queue(char* name){
	struct mq_attr attr;
	attr.mq_maxmsg = 10; //default system limit
	attr.mq_msgsize = MAX_MSG_SIZE+sizeof(msg_type);
	int qid = mq_open(name, O_RDONLY | O_CREAT, 0644, &attr);
	if(qid == -1){
		 printf("mq_open(): %s\n", strerror(errno));
		 exit(1);
	}
	mq_getattr(qid, &attr); // make the queue nonblocking
	attr.mq_flags = O_NONBLOCK;
	mq_setattr(qid, &attr, NULL);
	return qid;
}

int close_queue(char* name){
	int status = mq_unlink(name);
	if(status == -1){
		 printf("mq_unlink(): %s\n", strerror(errno));
	}
	return status;
}

#define send_request(qid, type, payload...) \
	struct message msg = {.mtype = type}; \
	typeof(msg.content.request.type) load = payload; \
	msg.content.request.type = load; \
	if(mq_send(qid, (char*) &msg, MAX_MSG_SIZE+sizeof(msg_type), type) == -1)perror("sending init name: ");

#define send_response(qid, type, payload...) \
	struct message tmp_msg = {.mtype = type}; \
	typeof(tmp_msg.content.response.type) load = payload; \
	tmp_msg.content.response.type = load; \
	mq_send(qid, (char*) &tmp_msg, MAX_MSG_SIZE+sizeof(msg_type), type);

#define send_init_name(qid, type, text...) \
	struct message tmp_msg = {.mtype=type}; \
	strcpy(tmp_msg.content.request.Init, text); \
	int status = mq_send(qid, (char*) &tmp_msg, MAX_MSG_SIZE+sizeof(msg_type), type); \
	if(status == -1)perror("sending init name: ");

#define send_message(qid, type, text...) \
	struct message tmp_msg = {.mtype=type}; \
	strcpy(tmp_msg.content.response.Message, text); \
	int status = mq_send(qid, (char*) &tmp_msg, MAX_MSG_SIZE+sizeof(msg_type), type); \
	if(status == -1)perror("message not sent: ");
