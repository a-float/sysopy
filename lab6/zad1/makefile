cc = gcc -Wall
server = ./server/server.c
client = ./client/client.c
all: main

server:
	@make rm_queues
	@$(cc) $(server) -o server.out
	@./server.out

client: $(client)
	@$(cc) $(client) -o client.out
	@./client.out

test: test.c
	@$(cc) test.c -o test.out
	@./test.out

rm_queues:
	ipcrm --all=msg

.PHONY: server client

clean:
	rm -Rf *.out
