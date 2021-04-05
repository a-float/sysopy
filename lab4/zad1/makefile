SHELL := /bin/bash
cc = gcc -Wall -std=c11 -g

zad1: zad1.c
	$(cc) zad1.c -o zad1.out
	./zad1.out

bbb: zad1_b.c
	$(cc) zad1_b.c -o zad1_b.out
	./zad1_b.out

.PHONY: clean zad1
clean:
	rm -Rf *.exe *.out
	