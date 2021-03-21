SHELL := /bin/bash
cc = gcc -Wall -std=c11 -g
line_count = 5000
res = pomiary_zad_4.txt

all:
	make big_data
	make test

zad4: zad4.c
	$(cc) zad4.c -o zad4.exe

zad4_sys: zad4.c
	$(cc) zad4.c -D sysIO -o zad4.exe

big_data:
	python make_data.py $(line_count) big_dane4.txt

run_test: zad4.exe
	printf "Male dane:\n" >> $(res)
	{ time ./zad4.exe dane4.txt res4.txt abc xxxxx 1>/dev/null ; } 2>> $(res)
	printf "\nDuze dane ($(line_count) linii):\n" >> $(res)
	{ time ./zad4.exe big_dane4.txt res4.txt abc xxxxx 1>/dev/null ; } 2>> $(res)

test: zad4.c big_dane4.txt
	printf "Systemowe IO:\n" > $(res)
	make zad4_sys
	make run_test
	printf "\nSTL IO:\n" >> $(res)
	make zad4
	make run_test

.PHONY: clean zad4
clean:
	rm -Rf *.exe res*.txt big_dane*.txt a.txt b.txt c.txt 
	