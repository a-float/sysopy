cc = gcc -Wall
out = wyniki_raw.txt

all: main

main:
	@$(cc) main.c -o main.out -lpthread

test_print:
	@$(cc) main.c -o main.out -lpthread -DDEBUG
	./main.out 3 blocks feep.pgm out.pgm 

test:
	@$(cc) main.c -o main.out -lpthread
	@./main.out 3 blocks lena.pgm out.pgm

bench: main
	echo '' > $(out)
	echo "BLOCKS" >> $(out)
	@./main.out 1 blocks lena.pgm out.pgm >> $(out)
	@./main.out 2 blocks lena.pgm out.pgm >> $(out)
	@./main.out 4 blocks lena.pgm out.pgm >> $(out)
	@./main.out 8 blocks lena.pgm out.pgm >> $(out)
	@./main.out 16 blocks lena.pgm out.pgm >> $(out)
	echo "NUMBERS" >> $(out)
	@./main.out 1 numbers lena.pgm out.pgm >> $(out)
	@./main.out 2 numbers lena.pgm out.pgm >> $(out)
	@./main.out 4 numbers lena.pgm out.pgm >> $(out)
	@./main.out 8 numbers lena.pgm out.pgm >> $(out)
	@./main.out 16 numbers lena.pgm out.pgm >> $(out)
.PHONY: server client

clean:
	rm -Rf *.out, wyniki_raw.txt out.pgm
