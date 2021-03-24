SHELL := /bin/bash
cc = gcc -Wall -std=c11 -g


zad3: zad3.c
	$(cc) zad3.c -o zad3.out

run: zad3
	sudo ./zad3.out /home/matt/Sysopy

.PHONY: clean zad1
clean:
	rm -Rf *.exe
	