#include "shared.h"

struct state {
  int   score[2];
  char* nicknames[2];
  char* symbols[2];
  char symbol;
  struct game_state game;
} state;

// TODO change to calloc
int connect_unix(char* path) {
  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path, sizeof(addr.sun_path));

  int sock = socket(AF_UNIX, SOCK_STREAM, 0);
  chck0(sock, "socket AF_UNIX: ", 20);
  int res = connect(sock, (struct sockaddr*) &addr, sizeof(addr));
  chck0(res, "connect AF_UNIX: ", 21);
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

int sock;
int main(int argc , char *argv[])
{
	//////////////////////////INPUT
	if (argc < 3) {
		printf("Usage [nick] [web|unix] [ip port|path]\n");
		exit(0);
	}
	if (strcmp(argv[2], "web") == 0 && argc == 5){
		sock = connect_web(argv[3], atoi(argv[4]));
	}
  	else if (strcmp(argv[2], "unix") == 0 && argc == 4){
  		sock = connect_unix(argv[3]);
  	}
  	else {
    	printf("Usage [nick] [web|unix] [ip port|path]\n");
    	exit(0);
  	}
	
	//////////////////////////SETUP
  	char* name = state.nicknames[0] = argv[1];
  	write(sock, name, strlen(name));

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
					// char second_input;
					// while ((second_input = getchar()) != EOR && x != '\n');
					printf("Your input is %s\n", c);
					message msg = { .type = msg_move };
					msg.payload.move = c[0];
					write(sock, &msg, sizeof(msg));
				}
			}
			else {  // socket event
				printf("Received a socket event\n");
			}
		}
	}
	return 0;
}