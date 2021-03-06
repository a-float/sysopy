#include "shared.h"

#define MAX_CONN 10
#define PING_INTERVAL 20

int epoll_fd;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct client {
  enum client_state { none = 0, init, waiting, playing } state;
  int fd;
  char name[NAME_SIZE];
  char marker;
  struct client* peer;
  struct game_state* game_state;
  bool pinging;
  bool active;
} client;

client clients[MAX_CONN];
client* client_with_no_pair = NULL;

typedef struct event_data {
  enum event_type { socket_event, client_event } type; // client_event = event from a client
  union payload { client* client; int socket; } payload;
} event_data;

void init_socket(int socket, void* addr, int addr_size){
	int res = bind(socket, (struct sockaddr*) addr, addr_size);
	chck0(res, "bind: ", 4);
	res = listen(socket, MAX_CONN);
	chck0(res, "listen: ", 5);
	struct epoll_event event = { .events = EPOLLIN | EPOLLPRI };
	event_data *event_data = calloc(sizeof(*event_data),1);
	event_data->type = socket_event;
	event_data->payload.socket = socket;
	event.data.ptr = event_data;
	printf("init socket %d\n", socket);
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &event);
}	

client* add_client(int client_fd){
	int real_client_fd = accept(client_fd, NULL, NULL);
	pthread_mutex_lock(&mutex);
	int i = MAX_CONN;
	while (i != -1){
		if(clients[i].state == none)break;
		i--;
	}
	if (i == -1) return NULL; // no place for the new client
	client* client = &clients[i];

	client->fd = real_client_fd;
	client->state = init;
	client->active = true;
	pthread_mutex_unlock(&mutex);
	return client;
}

void delete_client(client* client){
	printf("deleting user\n");
	if (client == client_with_no_pair){
		client_with_no_pair = NULL;
	}
	else if(client->peer){
		client->peer->peer = NULL;
		// client->peer->game_state = NULL;
		delete_client(client->peer); // kick the other player
		free(client->game_state);
		// client->peer = NULL;
		// client->game_state = NULL;
	}
	client->state = none;
	// client->name[0] = 0;
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->fd, NULL);
	close(client->fd);
	color_printf(YELLOW, "User %s has been removed\n", client->name);
}

void send_gamestate(client* client) {
  message msg = { .type = msg_game_state };
  memcpy(&msg.payload.state, client->game_state, sizeof(struct game_state));
  write(client->fd, &msg, sizeof msg);
}

void start_game(client* client1, client* client2) {
  client1->state = playing;
  client2->state = playing;
  client1->peer = client2;
  client2->peer = client1;

  client1->game_state = client2->game_state = calloc(1, sizeof(client1->game_state));
  memset(client1->game_state->board, '?', 9);

  message msg = { .type = msg_start_game };
  strncpy(msg.payload.start.name, client2->name, sizeof(client2->name));
  client1->marker = msg.payload.start.marker = 'x';
  write(client1->fd, &msg, sizeof msg);

  strncpy(msg.payload.start.name, client1->name, sizeof(client1->name));
  client2->marker = msg.payload.start.marker = 'O';
  write(client2->fd, &msg, sizeof msg);

  client1->game_state->move = client1->marker;
  send_gamestate(client1);
  send_gamestate(client2);
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
    if (client->game_state->board[i] == '-')
      return false;
  return true;
}

void handle_client_message(client* client) {
	if (client->state == init) {
		char new_client_name[NAME_SIZE];
		int name_length = read(client->fd, new_client_name, NAME_SIZE-1);
		pthread_mutex_lock(&mutex);
		bool is_the_name_taken = false;
		for(int i = 0; i > MAX_CONN; i++){
			if(strcmp(client->name, clients[i].name)==0){
				is_the_name_taken = true;
			}
		}
		if(!is_the_name_taken){
			strcpy(client->name, new_client_name);
			client->name[name_length] = '\0';
			printf("New client %s\n", client->name);
			if (client_with_no_pair) {
				printf("Connecting %s with %s\n", client->name, client_with_no_pair->name);
				start_game(client, client_with_no_pair);
				client_with_no_pair = NULL; // connected amd start the game
			} 
			else {
				printf("%s is waiting for an opponent...\n", client->name);
				message msg = { .type = msg_wait };
				write(client->fd, &msg, sizeof msg);
				client_with_no_pair = client;
				client->state = waiting; // connected but needs to wait for the opponent
			}
		} 
		else {
			message msg = { .type = msg_name_taken };
			printf("Username %s is already taken\n", client->name);
			write(client->fd, &msg, sizeof msg);
			strcpy(client->name, "new client");
			delete_client(client); // username taken
		}
		pthread_mutex_unlock(&mutex);
	}
	else {
		message msg;
		read(client->fd, &msg, sizeof msg);
		if (msg.type == msg_ping) {
			pthread_mutex_lock(&mutex);
			printf("pong %s\n", client->name);
			client->pinging = true;
			pthread_mutex_unlock(&mutex);
		} 
		else if (msg.type == msg_quit) {
			pthread_mutex_lock(&mutex);
			delete_client(client);
			pthread_mutex_unlock(&mutex);
		} 
		else if (msg.type == msg_move) {
			int move = msg.payload.move;
			if (client->game_state->move == client->marker 
			  && client->game_state->board[move] == '-'
			  && 0 <= move && move <= 8) {
			
				client->game_state->board[move] = client->marker;
				client->game_state->move = client->peer->marker;
				
				printf("%s moved\n", client->name);
				send_gamestate(client);
				send_gamestate(client->peer);

				if (check_game(client)) {
					msg.type = msg_win;
					msg.payload.win = client->marker;
				}
				else if (check_draw(client)) {
					msg.type = msg_win;
					msg.payload.win = '-';
				}
				if (msg.type == msg_win) {
					printf("Game between %s and %s finised\n", client->name, client->peer->name);
					client->peer->peer = NULL;
					write(client->peer->fd, &msg, sizeof msg);
					write(client->fd, &msg, sizeof msg);
				}
			} 
			else send_gamestate(client);
		}
	}
}

int main(int argc , char *argv[])
{
	if (argc != 3) {
		printf("Usage [port] [path]\n");
		exit(0);
	}
	int port = atoi(argv[1]);
	char* socket_path = argv[2];
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
	int local_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	chck0(local_sock, "socket AF_UNIX: ", 1);
	init_socket(local_sock, &local_addr, sizeof local_addr);

	// create and bind the internet socket
	int web_sock = socket(AF_INET, SOCK_STREAM, 0);
	chck0(local_sock, "socket AF)INET: ", 2);
	init_socket(web_sock, &web_addr, sizeof web_addr);

	// pthread_t ping_thread;
	// pthread_create(&ping_thread, NULL, ping, NULL);

	printf("Server live online (*:%d) and locally('%s')\n", port, socket_path);

	struct epoll_event events[10];
	while(true){
		int event_count = epoll_wait(epoll_fd, events, 10, -1);
		chck0(event_count, "epoll_wait: ", 3);
		for (int i = 0; i < event_count; i++){
			printf("an event has arrived\n");
			event_data* data = events[i].data.ptr;
			if (data->type == socket_event) {
				client* client = add_client(data->payload.socket);
				if (client == NULL){
					color_printf(RED, "Server is full\n");
					message response = { .type = msg_full };
					write(client->fd, &response, sizeof(response));
					close(client->fd);
					// could not add the client. Done with the event
				}
				else { // can add the client
					printf("Adding the client\n");
					event_data* event_data = calloc(sizeof(event_data), 1);
					event_data->type = client_event;
					event_data->payload.client = client;
					struct epoll_event event = { 
						.events = EPOLLIN | EPOLLET | EPOLLHUP,
						.data = { event_data }
					};
					// register the client
					printf("%d\n", client->fd);
					int res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client->fd, &event);
					printf("d is %d\n", res);
					chck0(res, "register client, epoll_ctl: ", 8);
					// color_printf(GREEN, "%s has connected\n", client->name);
				}
			}
			else if (data->type == client_event){
				printf("Received custom event\n");
				// client has disconnected
				if (events[i].events & EPOLLHUP) {
				  pthread_mutex_lock(&mutex);
				  delete_client(data->payload.client);
				  pthread_mutex_unlock(&mutex);
				}
				else {
					handle_client_message(data->payload.client);
				}
			}
			else{
				printf("I don't know what to do\n");
			}
		}
	}
	return 0;
}