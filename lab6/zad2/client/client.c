#include "../common.h"

key_t cid = -1;
qid_t qid = -1;
qid_t server_qid;
char client_name[30];
qid_t peer_qid = -1;

#define disconnect(peer_qid_ptr) \
	send_message(*peer_qid_ptr, Disconnect, ""); /*TODO maybe handle it differently?*/ \
	*peer_qid_ptr = -1; \
	send_request(server_qid, Disconnect, {.cid=cid}); \

void stop_client(int signo){
	if(peer_qid != -1){
		color_printf(CYAN, "\nLeaving the conversation\n");
		disconnect(&peer_qid);
	}
	color_printf(CYAN, "\nClosing the client\n");
	send_request(server_qid, Stop, {.cid = cid});
	close_queue(client_name);
	exit(0);
}

void parse_command(char* comm, qid_t server_qid, id_t cid, qid_t *peer_qid_ptr){
	qid_t peer_qid = *peer_qid_ptr;
	if(strcmp(comm, "LIST") == 0){
		send_request(server_qid, List, {.cid = cid});
	}
	else if(strncmp(comm, "CONNECT", sizeof("CONNECT")-1) == 0){
		char *token = strtok(comm, " ");
		token = strtok(NULL, " ");
		if(token == NULL){
			printf("Usage: CONNECT [peer_id]\n");
			return;
		}
		id_t peer_id = strtol(token, NULL, 10);
		color_printf(GREEN, "connecting to user with id %d\n", peer_id);
		send_request(server_qid, Connect, {.cid = cid, .peer_id=peer_id});
	}
	else if(strcmp(comm, "DISCONNECT") == 0){
		if(peer_qid != -1){
			disconnect(peer_qid_ptr);
			color_printf(YELLOW,"The conversation has ended\n"); \
		}
		else{
			color_printf(RED, "Cannot disconnect while not connected to anyone\n");
		}
	}
	else if(strcmp(comm, "STOP") == 0){
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
		char* peer_name = msg.content.response.Connect.peer_name; \
		color_printf(GREEN,"Started conversation with user %s\n", peer_name); \
		peer_qid = mq_open(peer_name, O_WRONLY); \
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
	send_init_name(server_qid, Init, client_name); \
	while(cid == -1){ \
		color_printf(GREEN, "waiting for the server...\n"); \
		while((status = mq_receive(qid, (char*) &recv_msg, MAX_MSG_SIZE+sizeof(msg_type), &priority)) != -1){ \
			cid = recv_msg.content.response.Init.cid; \
			color_printf(GREEN, "Succesfully connected to the server. Received id = %d\n", cid); \
		} \
		if(status == -1)perror("Could not receive because: ");\
		usleep(1 * 1000 * 1000); \
	} \
	int flags = fcntl(0, F_GETFL, 0); \
	fcntl(0, F_SETFL, flags | O_NONBLOCK); /*set up the stdin*/ 

int main(){
	struct message recv_msg;
	unsigned int priority = -1;
	printf("Input your name [/then at most 29 characters]:\n");
	scanf("%s", client_name);
	signal(SIGINT, stop_client);
	qid = create_queue(client_name);
	if(qid == -1){
		perror("Couldn't create the client queue: \n");
		exit(1); //TODO close some things
	}

	char* server_name = get_server_name();
	server_qid = mq_open(server_name, O_WRONLY);
	if(server_qid == -1){
		printf("Server is not live\n");
		stop_client(0);
	}
	free(server_name);

	color_printf(CYAN, "Starting the client. qid = %d\n", qid);
	init_client;
	struct mq_attr attr;
	char line[100];
	while(true){
		int r = read(0, &line, sizeof(line)-1);
		if(r > 0){
			line[r-1] = 0;
			// printf("%s (%d)\n", line, r);
			parse_command(line, server_qid, cid, &peer_qid);
		}
		mq_getattr(qid, &attr);
		// printf("Queue message count is %d\n", attr.mq_curmsgs);
		while(mq_receive(qid, (char*) &recv_msg, sizeof(recv_msg.content)+sizeof(msg_type), &priority) != -1){
			// printf("received a message of type %d\n", priority);
			handle_response(recv_msg);
		}	

		tick;
	}
	return 0;
}