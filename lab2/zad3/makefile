SHELL := /bin/bash
cc = gcc -Wall -std=c11 -g
line_count = 100000
res = pomiary_zad_3.txt

all:
	make big_data
	make test

zad3: zad3.c
	$(cc) zad3.c -lm -o zad3.exe

zad3_sys: zad3.c
	$(cc) zad3.c -D sysIO -lm -o zad3.exe

big_data:
	python make_data.py $(line_count) big_dane3.txt

run_test: zad3.exe
	printf "Male dane:\n" >> $(res)
	{ time ./zad3.exe dane.txt 1>/dev/null ; } 2>> $(res)
	printf "\nDuze dane ($(line_count) linii):\n" >> $(res)
	{ time ./zad3.exe big_dane3.txt 1>/dev/null ; } 2>> $(res)

test: zad3.c big_dane3.txt
	printf "Systemowe IO:\n" > $(res)
	make zad3_sys
	make run_test
	printf "\nSTL IO:\n" >> $(res)
	make zad3
	make run_test

.PHONY: clean zad3
clean:
	rm -Rf *.exe res*.txt big_dane*.txt a.txt b.txt c.txt
	