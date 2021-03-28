cc = gcc -Wall -std=c11 -g
ssize = 100
msize = 1000
bsize = 50000
spairs = 10
mpairs = 30
bpairs = 50
o_level = 0
flags = -D forked_merge

all: static_main

dirs:
	mkdir -p build
	mkdir -p data

static_lib: dirs lib.c
	$(cc) -c lib.c $(flags) -o build/lib.o
	ar rcs build/lib.a build/lib.o

#makes forked lib by default. pass flags='' to get normal merge
static_main: main.c static_lib
	$(cc) -c main.c -o build/main.o
	$(cc) build/main.o build/lib.a -o main.out -O$(o_level)


small_data: dirs
	python3 -c 'with open("data/small.txt", "w") as f: f.write(("abcefghijklmnopqrstuvwx "*3+"\n")*$(ssize))'

med_data: dirs
	python3 -c 'with open("data/medium.txt", "w") as f: f.write(("abcefghijklmnopqrstuvwx "*3+"\n")*$(msize))'

big_data: dirs
	python3 -c 'with open("data/big.txt", "w") as f: f.write(("abcefghijklmnopqrstuvwx "*3+"\n")*$(bsize))'

test_data: dirs small_data med_data big_data

test: main.out data/small.txt data/medium.txt data/big.txt
	bash ./test.sh $(spairs) $(ssize) $(mpairs) $(msize) $(bpairs) $(bsize)

.PHONY: clean
clean:
	rm -Rf ./build *.out *.so *.o *.a *.tmp
	rm -Rf ./data
