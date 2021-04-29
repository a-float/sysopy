#include "../common.h"

key_t cid = -1;
qid_t qid = -1;
qid_t server_qid;

void stop_client(int signo){
	color_printf(CYAN, "Closing the client\n");
	send_request(server_qid, Stop, {.cid = cid});
	close_queue(qid);
	exit(0);
}

#define disconnect \
	send_message(peer_qid, Disconnect, "");\
	peer_qid = -1; \
	send_request(server_qid, Disconnect, {.cid=cid}); \
	color_printf(YELLOW,"The conversation has ended\n");

#define wait_for_message(qid, types) while(msgrcv(qid, &msg, sizeof msg.content, types, IPC_NOWAIT) != -1)

void parse_command(char* comm, qid_t server_qid, id_t cid, qid_t peer_qid){
	if(strcmp(comm, "LIST") == 0){
		send_request(server_qid, List, {.cid = cid});
	}
	else if(strncmp(comm, "CONNECT", sizeof("CONNECT")-1) == 0){
		color_printf(GREEN, "connect to ...\n");
		char *token = strtok(comm, " ");
		token = strtok(NULL, " ");
		if(token == NULL){
			printf("Usage: CONNECT [peer_id]\n");
			return;
		}
		id_t peer_id = strtol(token, NULL, 10);
		color_printf(GREEN, "Connecting to user with id = %d\n", peer_id);
		send_request(server_qid, Connect, {.cid = cid, .peer_id=peer_id});
	}
	else if(strcmp(comm, "DISCONNECT") == 0){
		if(peer_qid != -1){
			disconnect;
		}
		else{
			color_printf(RED, "Cannot disconnect while not connected to anyone\n");
		}
	}
	else if(strcmp(comm, "STOP") == 0){
		if(peer_qid != -1){
			disconnect;
		}
		stop_client(0);
	}
	else if(peer_qid != -1){
		send_message(peer_qid, Message, comm);
	}
	else{
		color_printf(RED, "Unrecognized command '%s'\n", comm);
	}
}

#define handle_response(msg) \
	/*printf("Received a response of type %ld\n", msg.mtype);*/ \
	if(msg.mtype == List){ \
		color_printf(YELLOW, "%s", msg.content.response.Message); \
		fflush(stdout); \
	} \
	else if(msg.mtype == Connect){ \
		peer_qid = msg.content.response.Connect.peer_qid; \
		color_printf(GREEN,"Another user started a conversation with you\n"); \
	} \
	else if(msg.mtype == Message){ \
		color_printf(CYAN,"%s\n", msg.content.response.Message); \
	} \
	else if(msg.mtype == Error){ \
		color_printf(RED,"%s", msg.content.response.Message); \
	} \
	else if(msg.mtype == Stop){ \
		color_printf(CYAN, "The server is closing\n"); \
		stop_client(0); \
	} \
	else if(msg.mtype == Disconnect){ \
		peer_qid = -1; \
		send_request(server_qid, Disconnect, {.cid=cid}); \
		color_printf(YELLOW,"The conversation has ended\n"); \
	}

#define init_client \
	send_request(server_qid, Init, {.qid = qid}); \
	while(cid == -1){ \
		color_printf(GREEN, "waiting for the server...\n"); \
		wait_for_message(qid, Init){\
			cid = msg.content.response.Init.cid; \
			color_printf(GREEN, "Succesfully connected to the server. Received id = %d\n", cid); \
		} \
		usleep(1 * 1000 * 1000); \
	} \
	int flags = fcntl(0, F_GETFL, 0); \
	fcntl(0, F_SETFL, flags | O_NONBLOCK); /*set up the stdin*/ \
	signal(SIGINT, stop_client);

int main(){
	key_t key = get_server_key();
	server_qid = msgget(key, QUEUE_PERMISSIONS);
	qid = msgget(IPC_PRIVATE, QUEUE_PERMISSIONS);
	color_printf(CYAN, "Starting the client. qid = %d\n", qid);
	qid_t peer_qid = -1;
	init_client;
	
	char line[100];
	while(true){
		int r = read(0, &line, sizeof(line)-1);
		if(r > 0){
			line[r-1] = 0;
			// printf("%s (%d)\n", line, r);
			parse_command(line, server_qid, cid, peer_qid);
		}

		wait_for_message(qid, -100){ // gets all the messages with correct prioritizing
			handle_response(msg);
		}

		tick;
	}
	return 0;
}