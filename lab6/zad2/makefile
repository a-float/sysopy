cc = gcc -Wall
server = ./server/server.c
client = ./client/client.c

all: server client

server:
	@$(cc) $(server) -o server.out -lrt
	@./server.out

client: $(client)
	@$(cc) $(client) -o client.out -lrt
	@./client.out

test: test.c
	@$(cc) test.c -o test.out -lrt
	@./test.out

rm_mq_queues:
	rm /dev/mqueue/*

.PHONY: server client

clean:
	rm -Rf *.out
