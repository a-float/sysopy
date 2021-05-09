cc = gcc -Wall

all: main chef deliverer

main:
	@$(cc) main.c -o main.out -lrt -lpthread
deliverer:
	@$(cc) deliverer.c -o deliverer.out -lrt -lpthread
chef:
	@$(cc) chef.c -o chef.out -lrt -lpthread
test:
	@$(cc) chef.c -o chef.out -lrt -lpthread
	@$(cc) deliverer.c -o deliverer.out -lrt -lpthread
	@$(cc) main.c -o main.out -lrt -lpthread
	@./main.out 20 7

test: client server

.PHONY: server client

clean:
	rm -Rf *.out
