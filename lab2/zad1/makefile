SHELL := /bin/bash
cc = gcc -Wall -std=c11 -g
line_count = 20000

all:
	make big_data
	make test

zad1: zad1.c
	$(cc) zad1.c -o zad1.exe
# 	./zad1.exe dane1_a.txt dane1_b.txt

zad1_sys: zad1.c
	$(cc) zad1.c -D sysIO -o zad1.exe
# 	./zad1.exe dane1_a.txt dane1_b.txt

big_data:
	python3 -c 'with open("big_dane1_a.txt", "w") as f: f.write(("aaaabbbbbccccc "*3+"\n")*$(line_count))'
	python3 -c 'with open("big_dane1_b.txt", "w") as f: f.write(("xxxxxxxxxxyyyyyyyzzzzzz "*3+"\n")*$(line_count))'

run_test: zad1.exe
	printf "Male dane:\n" >> pomiar_zad_1.txt
	{ time ./zad1.exe dane1_a.txt dane1_b.txt 1>/dev/null ; } 2>> pomiar_zad_1.txt 
	printf "\nDuze dane ($(line_count) linii):\n" >> pomiar_zad_1.txt
	{ time ./zad1.exe big_dane1_a.txt big_dane1_b.txt 1>/dev/null ; } 2>> pomiar_zad_1.txt

test: zad1.c big_dane1_a.txt big_dane1_b.txt
	printf "Systemowe IO:\n" > pomiar_zad_1.txt
	make zad1_sys
	make run_test
	printf "\nLib IO:\n" >> pomiar_zad_1.txt
	make zad1
	make run_test

.PHONY: clean zad1
clean:
	rm -Rf *.exe res*.txt big_dane*.txt
# 	rm ./results/*
	