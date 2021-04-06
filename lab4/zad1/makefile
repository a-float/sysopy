cc = gcc -Wall -std=gnu99 -g

all: tester main

main: main.c
	$(cc) main.c -o main.out

tester: tester.c
	$(cc) tester.c -o tester.out
	$(cc) exec.c -o exec.out

test: all
	./main.out

clean:
	rm -Rf *.out
