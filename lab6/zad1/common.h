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

#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 100 

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

key_t get_server_key(){
	return ftok(SERVER_KEY_PATHNAME, PROJECT_ID);
}

int close_queue(qid_t qid){
	int status = msgctl(qid, IPC_RMID, NULL);
	if(status == -1){
		 printf("could not close the queue: %s\n", strerror(errno));
	}
	return status;
}

typedef enum msg_type {
	Stop = 1L,
	Disconnect,
	List,
	Init,
	Connect,
	Message,
	Error
} msg_type;

struct message {
	long mtype;
	union content {
		union request {
			struct {qid_t qid;} Init;
	 		struct {id_t cid;} Stop, List, Disconnect;
			struct {id_t cid; id_t peer_id;} Connect;
		} request;

		union response {
			struct {id_t cid;} Init, Stop, Disconnect;
			struct {qid_t peer_qid;} Connect;
			char Message[MAX_MSG_SIZE];
		} response;
	} content;
};


#define send_request(qid, type, payload...) \
	struct message msg = {.mtype = type}; \
	typeof(msg.content.request.type) load = payload; \
	msg.content.request.type = load; \
	msgsnd(qid, &msg, sizeof(load), 0);

#define send_response(qid, type, payload...) \
	struct message tmp_msg = {.mtype = type}; \
	typeof(tmp_msg.content.response.type) load = payload; \
	tmp_msg.content.response.type = load; \
	msgsnd(qid, &tmp_msg, sizeof(load), 0);

#define send_message(qid, type, text...) \
	struct message tmp_msg = {.mtype=type}; \
	strcpy(tmp_msg.content.response.Message, text); \
	msgsnd(qid, &tmp_msg, MAX_MSG_SIZE, 0);
