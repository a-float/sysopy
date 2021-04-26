#define SERVER
#include "tools.h"

bool done = false;
qid_t qid;
bool awaiting_closure = false;
client_list clients;

void close_server(int signo){
	bool no_clients = true;
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(clients.qids[i] != -1){
			send_message(clients.qids[i], Stop, ""); //TODO maybe fix this too
			no_clients = false;
		}
	}
	done = true;
	awaiting_closure = !no_clients; // if no clients present, dont wait
	if(awaiting_closure){
		color_printf(YELLOW, "Waiting for all the users to log out\n");
	}
}

int main(){
	struct message recv_msg;
	qid = create_queue();
	color_printf(GREEN, "Starting the server. qid = %d\n", qid);
	init_clients(&clients);
	signal(SIGINT, close_server);
	while(!done){
		while(msgrcv(qid, &recv_msg, sizeof recv_msg.content, -100, IPC_NOWAIT) != -1){
			// printf("Received request of type %ld\n", recv_msg.mtype);
			switch(recv_msg.mtype){
				case Init:
					handle_Init(recv_msg, &clients);
					break;
				case List:
					handle_List(recv_msg, &clients);
					break;
				case Connect:
					handle_Connect(recv_msg, &clients);
					break;
				case Stop:
					handle_Stop(recv_msg, &clients);
					break;
				case Disconnect:
					handle_Disconnect(recv_msg, &clients);
					break;
				default:
					color_printf(RED, "Error: unknown message type\n");
			}
		}

		tick;
	}
	while(awaiting_closure){ // only catching Stops
		while(msgrcv(qid, &recv_msg, sizeof recv_msg.content, Stop, IPC_NOWAIT) != -1){
			if(recv_msg.mtype == Stop){
				handle_Stop(recv_msg, &clients);
				bool no_clients = true;
				for(int i = 0; i < MAX_CLIENTS; i++){
					if(clients.qids[i] != -1){
						no_clients = false;
						break;
					}
				}
				if(no_clients)awaiting_closure = false;
			}
			else{
				color_printf(RED, "Error: unknown message type\n");
			}
		}
		tick;
	}
	close_queue(qid);
	color_printf(YELLOW, "\nClosing the server\n");
	return 0;
}