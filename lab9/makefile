cc = gcc -Wall

all: main

main:
	@$(cc) main.c -o main.out -lpthread

test:
	@$(cc) main.c -o main.out -lpthread
	./main.out

clean:
	rm -Rf *.out, wyniki_raw.txt out.pgm
