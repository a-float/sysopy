SHELL := /bin/bash
cc = gcc -Wall -std=c11 -g
flags = _USE_GNU

zad2: zad2.c
	$(cc) zad2.c -D$(flags) -o zad2.out
	./zad2.out

.PHONY: clean zad2
clean:
	rm -Rf *.exe *.out
	