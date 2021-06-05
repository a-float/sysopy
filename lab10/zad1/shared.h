#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/epoll.h>

#include <signal.h>
#include <pthread.h>

#define chck0(val, error, code) \
	if(val == -1){					\
		perror(error);			\
		exit(code);				\
	}
#define NAME_SIZE 20

#define RED "\033[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\033[93m"
#define CYAN "\033[96m"
#define RESET_COL "\033[0m"

#define color_printf(color, content...) \
printf(color);                        \
printf(content);                      \
printf(RESET_COL);                    \
fflush(stdout);


struct game_state { char move; char board[9]; };

typedef struct message {
	enum msg_type {
		msg_ping,
		msg_name_taken,
		msg_wait,
		msg_full,
		msg_start_game,
		msg_game_state,
		msg_move,
		msg_quit,
		msg_win,
		msg_lose
	} type;
	union msg_payload {
		struct { 
			char name[NAME_SIZE];
			char marker;
		} start;
		int move;
		struct game_state state;
		char win;
	} payload;
} message;