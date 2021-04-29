#include "../common.h"

typedef struct client_list{
	qid_t qids[MAX_CLIENTS];
	bool busy[MAX_CLIENTS];
} client_list;


int create_queue(){
	key_t server_key = get_server_key();
	qid_t qid = msgget(server_key, IPC_CREAT | IPC_EXCL | QUEUE_PERMISSIONS);
	if(qid == -1){
		printf("could not create the queue: %s\n", strerror(errno));
		exit(-1);
	}
	return qid;
}

void init_clients(client_list *clients){
	for(int i = 0; i < MAX_CLIENTS; i++){
		clients->qids[i] = -1;
		clients->busy[i] = false;
	}
}

key_t add_client(qid_t qid, client_list *clients){
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(clients->qids[i] == -1){
			clients->qids[i] = qid;
			clients->busy[i] = false;
			return i;
		}
	}
	return -1;
}

void handle_Init(struct message msg, client_list *clients){
	qid_t client_qid = msg.content.request.Init.qid;
	key_t cid = add_client(client_qid, clients);
	if(cid == -1){
		send_message(client_qid, Error, "Server is currently full. Please try again later\n");
		return;
	}
	send_response(client_qid, Init, {.cid = cid});
	color_printf(CYAN,"User %d logs in\n", cid);
}

void handle_List(struct message msg, client_list *clients){
	id_t cid  = msg.content.request.List.cid;
	qid_t client_qid = clients->qids[cid];
	int max_list_size = MAX_CLIENTS*35;
	char *list = calloc(sizeof(char),max_list_size);
	int cx = 0;
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(clients->qids[i] == -1) continue;
		cx += snprintf(list+cx, max_list_size-cx, "User %d : %s\n", i, clients->busy[i] ? "busy" : "available");
	}
	color_printf(YELLOW,"User %d requests the list of the users\n", cid);
	send_message(client_qid, List, list);
	free(list);
}

void handle_Connect(struct message msg, client_list *clients){
	id_t client_id = msg.content.request.Connect.cid;
	id_t peer_id = msg.content.request.Connect.peer_id;
	qid_t client_qid = clients->qids[client_id];
	qid_t peer_qid;
	if(peer_id < 0 || peer_id > MAX_CLIENTS || clients->qids[peer_id] == -1){
		send_message(client_qid, Error, "Connection failed. User with specified id doesn't exist\n");
		return;
	}
	if(peer_id == client_id){
		send_message(client_qid, Error, "Creating a loop connection\n");
	}
	if(clients->busy[peer_id]){
		send_message(client_qid, Error, "Connection failed. Selected user is currently busy\n");
		return;
	}
	else{
		peer_qid = clients->qids[peer_id];
	}
	struct message out_msg = {.mtype = Connect};
	out_msg.content.response.Connect.peer_qid = peer_qid;
	size_t load_size = sizeof(out_msg.content.response.Connect);
	msgsnd(client_qid, &out_msg, load_size, 0);
	out_msg.content.response.Connect.peer_qid = client_qid;
	msgsnd(peer_qid, &out_msg, load_size, 0);
	
	color_printf(GREEN,"User %d requests connection with user %d\n", client_id, peer_id);
	clients->busy[peer_id] = true;
	clients->busy[client_id] = true;
}

void handle_Stop(struct message msg, client_list *clients){
	id_t cid = msg.content.request.Stop.cid;
	clients->qids[cid] = -1;
	clients->busy[cid] = false;
	color_printf(CYAN,"User %d logs out\n", cid);
}

void handle_Disconnect(struct message msg, client_list *clients){
	id_t cid = msg.content.request.Connect.cid;
	clients->busy[cid] = false;
	color_printf(YELLOW,"User %d leaves the conversation\n", cid);
}