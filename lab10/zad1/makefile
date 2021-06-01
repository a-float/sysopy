cc = gcc -Wall

all: server client

server:
	@$(cc) server.c -o server.out -lpthread
# 	@./server.out

client:
	@$(cc) client.c -o client.out -lpthread
# 	@./client.out

rem_unix:
	rm unix*

clean:
	rm -Rf *.out
