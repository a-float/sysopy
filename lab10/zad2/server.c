#include "shared.h"

#define MAX_CONN 10
#define PING_COOLDOWN 5

union addr {
	struct sockaddr_un uni;
	struct sockaddr_in web;
};
typedef struct sockaddr* sa;

int epoll_fd;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct client {
	char name[NAME_SIZE];
	char marker;
	struct client* peer;
	struct game_state* game_state;
	bool pinging;
	union addr addr;
	int sock, addrlen;
	enum client_state { none = 0, waiting, playing } state;
} client;

client clients[MAX_CONN];
client* client_with_no_pair = NULL;

void init_socket(int socket, void* addr, int addr_size){
	int res = bind(socket, (struct sockaddr*) addr, addr_size);
	chck0(res, "bind: ", 4);
	struct epoll_event event = { 
		.events = EPOLLIN | EPOLLPRI, 
		.data = { .fd = socket } 
	};
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &event);
}

void send_gamestate(client* client) {
  	color_printf(CYAN, "sending gamestate to %s\n", client->name);
	message msg = { .type = msg_game_state };
	memcpy(&msg.payload.state, client->game_state, sizeof(struct game_state));
	sendto(client->sock, &msg, sizeof(msg), 0, (sa) &client->addr, client->addrlen);
}

void start_game(client* client1, client* client2) {
	client1->state = playing;
	client2->state = playing;
	client1->peer = client2;
	client2->peer = client1;

	client1->game_state = client2->game_state = calloc(1, sizeof(client1->game_state));
	for(int i = 1; i <= 9; i++){
		client1->game_state->board[i-1] = '0'+i;
	}

	message msg = { .type = msg_start_game };
	strncpy(msg.payload.start.name, client2->name, sizeof(client2->name));
	client1->marker = msg.payload.start.marker = 'x';
	sendto(client1->sock, &msg, sizeof(msg), 0, (sa) &client1->addr, client1->addrlen);

	strncpy(msg.payload.start.name, client1->name, sizeof(client1->name));
	client2->marker = msg.payload.start.marker = 'o';
	sendto(client2->sock, &msg, sizeof(msg), 0, (sa) &client2->addr, client2->addrlen);

	client1->game_state->move = client1->marker;
	send_gamestate(client1);
	send_gamestate(client2);
}

void add_client(union addr* addr, socklen_t addrlen, int sock, char* name) {
	color_printf(RED, "Adding %s\n", name);
	pthread_mutex_lock(&mutex);
	int new_client_index = -1; // i is the first free clients array index
	for(int i = 0; i < MAX_CONN; i++){
		if(clients[i].state == none){
			new_client_index = i;
		}
		else if(strcmp(name, clients[i].name) == 0){  // the name is already taken
			pthread_mutex_unlock(&mutex);
			message msg = { .type = msg_name_taken };
			printf("Name %s already taken\n", name);
			sendto(sock, &msg, sizeof msg, 0, (sa) addr, addrlen);
			return;
		}
	}
	if (new_client_index == -1) {
	    pthread_mutex_unlock(&mutex);
	    printf("Server is full\n");
	    message msg = { .type = msg_full };
	    sendto(sock, &msg, sizeof msg, 0, (sa) addr, addrlen);
	    return;
	  }
	client* client = &clients[new_client_index];
	memcpy(&client->addr, addr, addrlen);
	client->sock = sock;
	client->state = waiting;
	client->addrlen = addrlen;
	client->pinging = true;
	
	printf("%s joined the server\n", name);

	for(int i = 0; i < NAME_SIZE-1; i++){
		client->name[i] = name[i];
	}
	client->name[NAME_SIZE-1] = 0;
	// strncpy(client->name, name, sizeof(client->name) - 1);
	color_printf(GREEN, "name is %s\n", client->name);
	if (client_with_no_pair) {
		printf("Connecting %s with %s\n", client->name, client_with_no_pair->name);
		start_game(client_with_no_pair, client);
		client_with_no_pair = NULL;
	} 
	else {
		printf("%s is waiting\n", client->name);
		client_with_no_pair = client;
		message msg = { .type = msg_wait };
		sendto(client->sock, &msg, sizeof msg, 0, (sa) &client->addr, client->addrlen);
	}
	pthread_mutex_unlock(&mutex);
}

void delete_client(client* client){
	if (client == client_with_no_pair){
		client_with_no_pair = NULL;
	}
	else if(client->peer){
		client->peer->peer = NULL;
		client->peer->game_state = NULL;
		delete_client(client->peer); // kick the other player
		free(client->game_state);
		client->peer = NULL;
		client->game_state = NULL;
	}
	message msg = { .type = msg_quit };
	sendto(client->sock, &msg, sizeof msg, 0, (sa) &client->addr, client->addrlen);
	memset(&client->addr, 0, sizeof(client->addr));
	client->state = none;
	client->sock = 0;
	color_printf(YELLOW, "User %s has been removed\n", client->name);
}

bool check_game(client* client) {
 	static int win[8][3] = {
   	 {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6},
   	 {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}
  	};
	char* board = client->game_state->board;
	char c = client->marker;
	for (int i = 0; i < 8; i++) {
    if (board[win[i][0]] == c &&
        board[win[i][1]] == c && 
        board[win[i][2]] == c) 
        return true;
  	}
  	return false;
}

bool check_draw(client* client) {
	for (int i = 0; i < 9; i++)
		if (client->game_state->board[i] != 'o' && client->game_state->board[i] != 'x'){
			return false;
		}
	return true;
}

void* ping(void* _) {
	message msg = { .type = msg_ping };
	while(true){
	    sleep(PING_COOLDOWN);
	    pthread_mutex_lock(&mutex);
    	printf("Pinging clients\n");
    	for (int i = 0; i < MAX_CONN; i++) {
      		if (clients[i].state != none) {
        		if (clients[i].pinging) {
          			clients[i].pinging = false;
          			sendto(clients[i].sock, &msg, sizeof msg, 0, (sa) &clients[i].addr, clients[i].addrlen);
        		}
       			else delete_client(&clients[i]);
      		}
    	}
    	pthread_mutex_unlock(&mutex);
  	}
}

void handle_client_message(client* client, message* msg) {
	if (msg->type == msg_ping) {
		pthread_mutex_lock(&mutex);
		printf("pong %s\n", client->name);
		client->pinging = true;
		pthread_mutex_unlock(&mutex);
	} 
	else if (msg->type == msg_quit) {
		color_printf(RED, " user has disconnected \n");
		pthread_mutex_lock(&mutex);
		delete_client(client);
		pthread_mutex_unlock(&mutex);
	} 
	else if (msg->type == msg_move) {
		printf("Received a move msg %d\n", msg->payload.move);
		int move = msg->payload.move;
		if (client->game_state->move == client->marker 
		  && client->game_state->board[move-1] != 'o'
		  && client->game_state->board[move-1] != 'x'
		  && 1 <= move && move <= 9) {
		
			client->game_state->board[move-1] = client->marker;
			client->game_state->move = client->peer->marker;
			
			printf("%s moved\n", client->name);
			send_gamestate(client);
			send_gamestate(client->peer);

			if (check_game(client)) {
				msg->type = msg_win;
				msg->payload.win = client->marker;
			}
			else if (check_draw(client)) {
				msg->type = msg_win;
				msg->payload.win = '-';
			}
			if (msg->type == msg_win) {
				printf("Game %s vs %s has ended\n", client->peer->name, client->name);
				client->peer->peer = NULL;
				sendto(client->peer->sock, msg, sizeof(msg), 0, (sa) &client->peer->addr, client->peer->addrlen);
		        sendto(client->sock, msg, sizeof(msg), 0, (sa) &client->addr, client->addrlen);
		        delete_client(client);
			}
		} 
		else{
			color_printf(RED, "Invalid move\n");
			send_gamestate(client);
		}
	}
}

int local_sock, web_sock;
char* socket_path;
void on_sigint(int _){
	printf("closing from handler\n");
	close(web_sock);
	close(local_sock);
	unlink(socket_path);
	exit(0);
}

int main(int argc , char *argv[])
{
	if (argc != 3) {
		printf("Usage [port] [path]\n");
		exit(0);
	}
	int port = atoi(argv[1]);
	socket_path = argv[2];
	epoll_fd = epoll_create1(0);
	chck0(epoll_fd, "epoll_create1: ", 7);

	// unix socket
	struct sockaddr_un local_addr = { .sun_family = AF_UNIX };
	strncpy(local_addr.sun_path, socket_path, sizeof local_addr.sun_path);
	
	// web socket
	struct sockaddr_in web_addr = {
	.sin_family = AF_INET, .sin_port = htons(port),
	.sin_addr = { .s_addr = htonl(INADDR_ANY) },
  };

	// create and bind the UNIX socket
	local_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	chck0(local_sock, "socket AF_UNIX: ", 1);
	init_socket(local_sock, &local_addr, sizeof local_addr);

	// create and bind the internet socket
	web_sock = socket(AF_INET, SOCK_DGRAM, 0);
	chck0(local_sock, "socket AF)INET: ", 2);
	init_socket(web_sock, &web_addr, sizeof web_addr);

	signal(SIGINT, on_sigint);

	pthread_t ping_thread;
	pthread_create(&ping_thread, NULL, ping, NULL);

	printf("Server live online (*:%d) and locally('%s')\n", port, socket_path);

	struct epoll_event events[10];
	while(true){
		int event_count = epoll_wait(epoll_fd, events, 10, -1);
		chck0(event_count, "epoll_wait: ", 5);
		for (int i = 0; i < event_count; i++){
			printf("Event!\n");
			union addr addr;
			message msg;
			int sock = events[i].data.fd;			
			socklen_t addrlen = sizeof addr;
			recvfrom(sock, &msg, sizeof msg, 0, (sa) &addr, &addrlen);
			printf("Message %ld\n", sizeof(msg));
			if (msg.type == msg_connect) {
				printf("adding client\n");
	        	add_client(&addr, addrlen, sock, msg.payload.name);
	      	} 
	      	else {
	      		client sender;
	      		for(int j = 0; j < MAX_CONN; j++){
	      			if(memcmp(&clients[i].addr, &addr, addrlen) == 0){
	      				sender = clients[i];
	      				break;
	      			}
	      		}
	        	handle_client_message(&sender, &msg);
			}
		}
	}
	return 0;
}