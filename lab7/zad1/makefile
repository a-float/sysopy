cc = gcc -Wall
server = ./shm_server.c
client = ./shm_client.c

all: server client

test:
	@$(cc) main.c -o main
	@$(cc) chef.c -o chef.out
	@./main

server: $(server)
	@$(cc) $(server) -o server.out
# 	@./server.out

client: $(client)
	@$(cc) $(client) -o client.out
# 	@./client.out

test: client server

.PHONY: server client

clean:
	rm -Rf *.out
