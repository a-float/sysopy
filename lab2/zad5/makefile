SHELL := /bin/bash
cc = gcc -Wall -std=c11 -g
line_count = 10000
res = pomiary_zad_5.txt

all:
	make big_data
	make test
	
zad5: zad5.c
	$(cc) zad5.c -o zad5.exe

zad5_sys: zad5.c
	$(cc) zad5.c -D sysIO -o zad5.exe

big_data:
	python make_data.py $(line_count) big_dane5.txt

run_test: zad5.exe
	printf "Male dane:\n" >> $(res)
	{ time ./zad5.exe dane5.txt res5.txt 1>/dev/null ; } 2>> $(res)
	printf "\nDuze dane ($(line_count) linii):\n" >> $(res)
	{ time ./zad5.exe big_dane5.txt res5.txt 1>/dev/null ; } 2>> $(res)

test: zad5.c big_dane5.txt
	printf "Systemowe IO:\n" > $(res)
	make zad5_sys
	make run_test
	printf "\nSTL IO:\n" >> $(res)
	make zad5
	make run_test

.PHONY: clean zad5
clean:
	rm -Rf *.exe res*.txt big_dane*.txt a.txt b.txt c.txt
	