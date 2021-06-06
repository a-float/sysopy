#include "shared.h"

struct state {
  char* names[2];
  char markers[2];
  char marker;
  struct game_state game;
} state;

int sock;

const char board[] = 
"			\n"
" %c │ %c │ %c \n"
"───┼───┼───\n"
" %c │ %c │ %c \n"
"───┼───┼───\n"
" %c │ %c │ %c \n"
" 			\n"
"Your name:    %c - %s\n"
"Your opponent: %c - %s\n"
"Current turn: 	%c\n\n";

void draw_board() {
	char* b = state.game.board;
	printf(board, b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7], b[8],
			state.markers[0], state.names[0], 
			state.markers[1], state.names[1],
			state.game.move
	);
	printf("Your move:");
	fflush(stdout);
}

// TODO remove addr
int connect_unix(char* path, char* user) {
  struct sockaddr_un addr, bind_addr;
  memset(&addr, 0, sizeof(addr));
  bind_addr.sun_family = AF_UNIX;
  addr.sun_family = AF_UNIX;
  snprintf(bind_addr.sun_path, sizeof bind_addr.sun_path, "./%s%ld", user, time(NULL));
  strncpy(addr.sun_path, path, sizeof addr.sun_path);

  int sock = socket(AF_UNIX, SOCK_DGRAM, 0); 
  chck0(sock, "socket: ", 1);
  int res = bind(sock, (void*) &bind_addr, sizeof addr);
  chck0(res, "socket: ", 2);
  res = connect(sock, (struct sockaddr*) &addr, sizeof addr);
  chck0(res, "connect: ", 3);

  return sock;
}
int connect_web(char* ipv4, int port) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, ipv4, &addr.sin_addr) <= 0) {
    printf("Invalid address\n");
    exit(0);
  }

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  chck0(sock, "socket AF_INET: ", 22);
  connect(sock, (struct sockaddr*) &addr, sizeof(addr));
  chck0(sock, "socket AF_INET: ", 23);
  return sock;
}


void handle_server_message(message msg){
	if (msg.type == msg_wait) {
		printf("Waiting for an opponent\n");
    } 
    else if (msg.type == msg_start_game) {  // start game
    	state.names[1] = calloc(1, sizeof(msg.payload.start.name));
		memcpy(state.names[1], &msg.payload.start.name, sizeof(msg.payload.start.name));
		state.markers[1] = msg.payload.start.marker == 'o' ? 'x' : 'o';
		state.marker = msg.payload.start.marker;
		state.markers[0] = msg.payload.start.marker == 'o' ? 'o' : 'x';
    } 
    else if (msg.type == msg_name_taken) {
        printf("This username is already taken\n");
        close(sock);
        exit(0);
    } 
    else if (msg.type == msg_quit){
        printf("Quitting\n");
        close(sock);
        exit(0);
    }
    else if (msg.type == msg_full) {
		printf("Server is full\n");
		close(sock);
		exit(0);
    } 
    else if (msg.type == msg_ping){ // activity check
        printf("Im pinged\n");
        write(sock, &msg, sizeof msg); // write the ping message back
    }
    else if (msg.type == msg_game_state) { // got the new game state
		memcpy(&state.game, &msg.payload.state, sizeof state.game);
		// render_update();
		draw_board();
    } 
    else if (msg.type == msg_win) { // end of the game
        if (msg.payload.win == state.marker) printf("\r You won! ^^\n\n");
        else if (msg.payload.win == '-') printf("\r No one has won.\n\n");
        else printf("\r You lost :c\n\n");
        close(sock);
        exit(0);
    }
}

void on_sigint(int _) {
  message msg = { .type = msg_quit };
  free(state.names[1]);
  send(sock, &msg, sizeof msg, 0);
  exit(0);
}

int main(int argc , char *argv[])
{
	//////////////////////////INPUT
    char* name;
	if (argc < 3) {
		printf("Usage [nick] [web|unix] [ip port|path]\n");
		exit(0);
	}
    else {
        name = state.names[0] = argv[1];
        if (strcmp(argv[2], "web") == 0 && argc == 5){
            sock = connect_web(argv[3], atoi(argv[4]));
        }
        else if (strcmp(argv[2], "unix") == 0 && argc == 4){
            sock = connect_unix(argv[3], name);
        }
        else{
            printf("Usage [nick] [web|unix] [ip port|path]\n");
            exit(0);
        }
    }
	
	//////////////////////////SETUP
	signal(SIGINT, on_sigint);
  	message msg = { .type = msg_connect };
    strncpy(msg.payload.name, name, sizeof(msg.payload.name));
    send(sock, &msg, sizeof msg, 0);

  	int epoll_fd = epoll_create1(0);
  	chck0(epoll_fd, "epoll_create1: ", 25);

  	// register STDIN
  	struct epoll_event stdin_event = { 
	    .events = EPOLLIN | EPOLLPRI,
	    .data = { .fd = STDIN_FILENO }
	  };
  	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_event);
  	
  	// register server events
	struct epoll_event socket_event = { //epollhup not needed? 
		.events = EPOLLIN | EPOLLPRI | EPOLLHUP,
		.data = { .fd = sock }
	};
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &socket_event);
	char c[10]; // user move choice
	struct epoll_event events[2];
	while(true){
		int event_count = epoll_wait(epoll_fd, events, 2, 1);
		for(int i = 0; i < event_count; i++){
			if (events[i].data.fd == STDIN_FILENO) { // keyboard event
				if (scanf("%s", c) != -1){
					message msg = { .type = msg_move };
					msg.payload.move = c[0] - '0';
					printf("Sending move %c\n", c[0]);
					sendto(sock, &msg, sizeof(msg), 0, NULL, sizeof(struct sockaddr_in));
				}
			}
			else {  // socket event
				message msg;
                recvfrom(sock, &msg, sizeof(msg), 0, NULL, NULL);
        		read(sock, &msg, sizeof msg);
				if (events[i].events & EPOLLHUP) { // server disconnected
			        printf("Disconnected from the server\n\n");
			        on_sigint(0);
			    } 
				handle_server_message(msg);
			}
		}
	}
	return 0;
}