cc = gcc -Wall

all: main

main: main.c
	$(cc) main.c -o main.out
	./main.out file.txt

test: all
	./main.out

clean:
	rm -Rf *.out
