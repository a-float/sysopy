cc = gcc -Wall -std=c11 -g

all: shared_lib static_lib

dirs:
	mkdir -p build

static_lib: dirs lib.c
	$(cc) -c lib.c -o build/lib.o
	ar rcs build/lib.a build/lib.o

shared_lib: dirs lib.c
	$(cc) -c -fPIC lib.c -o build/lib.o
	$(cc) -shared -fPIC -o libmerge.so build/lib.o

static_main: main.c static_lib
	$(cc) -c main.c -o build/main.o
	$(cc) build/main.o build/lib.a -o main.out

shared_main: main.c shared_lib
	$(cc) main.c -L. -o main.out -lmerge -Wl,-rpath=.

.PHONY: clean
clean:
	rm -Rf build *.out *.so *.o *.a *.tmp
