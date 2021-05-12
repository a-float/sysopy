cc = gcc -Wall

all: main chef deliverer

main:
	@$(cc) main.c -o main.out -lrt -lpthread

test_print:
	@$(cc) main.c -o main.out -lrt -lpthread -DDEBUG
	./main.out 3 blocks hands.pgm out.pgm 

test:
	@$(cc) main.c -o main.out -lrt -lpthread
	./main.out 10 blocks lena.pgm out.pgm

.PHONY: server client

clean:
	rm -Rf *.out
