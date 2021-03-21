SHELL := /bin/bash
cc = gcc -Wall -std=c11 -g
line_count = 100000
res = pomiary_zad_2.txt

all:
	make big_data
	make test

zad2: zad2.c
	$(cc) zad2.c -o zad2.exe
# 	./zad2.exe o zad2/dane2.txt

zad2_sys: zad2.c
	$(cc) zad2.c -D sysIO -o zad2.exe
# 	./zad1.exe dane1_a.txt dane1_b.txt

big_data:
	python3 -c 'with open("big_dane2.txt", "w") as f: f.write(("bbbbbbbbbbbbbbbb\n")*$(line_count))'
	python3 -c 'with open("big_dane2.txt", "w") as f: f.write(("aaaaaaaaaaaaaxaa\n")*$(line_count))'

run_test: zad2.exe
	printf "Male dane:\n" >> $(res)
	{ time ./zad2.exe x dane2.txt 1>/dev/null ; } 2>> $(res)
	printf "\nDuze dane ($(line_count) linii):\n" >> $(res)
	{ time ./zad2.exe x big_dane2.txt 1>/dev/null ; } 2>> $(res)

test: zad2.c big_dane2.txt
	printf "Systemowe IO:\n" > $(res)
	make zad2_sys
	make run_test
	printf "\nSTL IO:\n" >> $(res)
	make zad2
	make run_test

.PHONY: clean zad2
clean:
	rm -Rf *.exe res*.txt big_dane*.txt
	