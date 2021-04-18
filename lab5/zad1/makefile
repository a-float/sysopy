cc = gcc -Wall

all: main

main: main.c
	$(cc) main.c -o main.out

test: all
	./main.out

clean:
	rm -Rf *.out
