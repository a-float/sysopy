#include "../common.h"

typedef struct client_list{
	qid_t qids[MAX_CLIENTS];
	bool busy[MAX_CLIENTS];
	char names[MAX_CLIENTS][MAX_NAME_SIZE];
} client_list;


void init_clients(client_list *clients){
	for(int i = 0; i < MAX_CLIENTS; i++){
		clients->qids[i] = -1;
		clients->busy[i] = false;
	}
}

key_t add_client(char* client_name, client_list *clients){
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(clients->qids[i] == -1){
			clients->qids[i] = mq_open(client_name, O_WRONLY);
			clients->busy[i] = false;
			strcpy(clients->names[i], client_name);
			return i;
		}
	}
	return -1;
}

#define cid_is_valid(cid) (cid >= 0 && cid < MAX_CLIENTS && clients->qids[cid] != -1)


void handle_Init(struct message msg, client_list *clients){
	char* client_name = msg.content.request.Init;
	key_t cid = add_client(client_name, clients);
	printf("given cid is %d\n",cid);
	qid_t client_qid = clients->qids[cid];
	if(cid == -1){
		send_message(client_qid, Error, "Server is currently full. Please try again later\n");
		return;
	}
	printf("sending id = %d to client with qid %d\n", cid, client_qid);
	send_response(client_qid, Init, {.cid = cid});
	color_printf(CYAN,"User %d:%s logs in\n", cid, client_name);
}

void handle_List(struct message msg, client_list *clients){
	id_t cid  = msg.content.request.List.cid;
	printf("cid is %d\n", cid);
	if(!cid_is_valid(cid))return;
	qid_t client_qid = clients->qids[cid];
	int max_list_size = MAX_CLIENTS*35;
	char *list = calloc(sizeof(char),max_list_size);
	int cx = 0;
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(clients->qids[i] == -1) continue;
		cx += snprintf(list+cx, max_list_size-cx, "User %d. %s : %s\n", i, clients->names[i], clients->busy[i] ? "busy" : "available");
	}
	printf("LIST is '%s'\n", list);
	color_printf(YELLOW,"User %d requests the list of the users\n", cid);
	send_message(client_qid, List, list);
	free(list);
}

void handle_Connect(struct message msg, client_list *clients){
	id_t client_id = msg.content.request.Connect.cid;
	id_t peer_id = msg.content.request.Connect.peer_id;
	printf("cid is %d peer id is %d\n", client_id, peer_id);
	if(!cid_is_valid(client_id))return;
	qid_t client_qid = clients->qids[client_id];
	qid_t peer_qid;
	if(peer_id < 0 || peer_id > MAX_CLIENTS || clients->qids[peer_id] == -1){
		send_message(client_qid, Error, "Connection failed. User with specified id doesn't exist\n");
		return;
	}
	if(clients->busy[peer_id]){
		send_message(client_qid, Error, "Connection failed. Selected user is currently busy\n");
		return;
	}
	if(peer_id == client_id){
		send_message(client_qid, Error, "Can't create a loop connection\n");
		return;
	}
	
	else{
		peer_qid = clients->qids[peer_id];
	}
	struct message out_msg = {.mtype = Connect};
	strcpy(out_msg.content.response.Connect.peer_name, clients->names[peer_id]);
	size_t load_size = sizeof(out_msg.content.response.Connect);
	mq_send(client_qid, (char*) &out_msg, load_size + sizeof(msg_type), Connect);
	strcpy(out_msg.content.response.Connect.peer_name, clients->names[client_id]);
	mq_send(peer_qid, (char*) &out_msg, load_size + sizeof(msg_type), Connect);
	
	color_printf(GREEN,"User %d requests connection with user %d\n", client_id, peer_id);
	clients->busy[peer_id] = true;
	clients->busy[client_id] = true;
}
void handle_Stop(struct message msg, client_list *clients){
	id_t cid = msg.content.request.Stop.cid;
	printf("cid is %d\n", cid);
	if(!cid_is_valid(cid))return;
	mq_close(clients->qids[cid]);
	clients->qids[cid] = -1;
	clients->busy[cid] = false;
	color_printf(CYAN,"User %d logs out\n", cid);
}

void handle_Disconnect(struct message msg, client_list *clients){
	id_t cid = msg.content.request.Connect.cid;
	printf("cid is %d\n", cid);
	if(!cid_is_valid(cid))return;
	clients->busy[cid] = false;
	color_printf(YELLOW,"User %d leaves the conversation\n", cid);
}