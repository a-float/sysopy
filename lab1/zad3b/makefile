cc = gcc -Wall -std=c11 -g
ssize = 100
msize = 1000
bsize = 10000
spairs = 5
mpairs = 25
bpairs = 100
o_level = 0

all: static_main

dirs:
	mkdir -p build
	mkdir -p data

static_lib: dirs lib.c
	$(cc) -c lib.c -o build/lib.o
	ar rcs build/lib.a build/lib.o

shared_lib: dirs lib.c
	$(cc) -c -fPIC lib.c -o build/lib.o
	$(cc) -shared -fPIC -o libmerge.so build/lib.o

static_main: main.c static_lib
	$(cc) -c main.c -o build/main.o
	$(cc) build/main.o build/lib.a -o main.out -O$(o_level)

shared_main: main.c shared_lib
	$(cc) main.c -L. -o main.out -lmerge -Wl,-rpath=. O$(o_level)

dynamic_main: main.c shared_lib
	$(cc) main.c -o main.out -ldl -D dynamic O$(o_level)


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