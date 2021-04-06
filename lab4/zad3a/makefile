cc = gcc -Wall -std=gnu99 -g
n = 10000

all: catcher sender

catcher: catcher.c
	$(cc) catcher.c -o catcher.out

sender: sender.c
	$(cc) sender.c -o sender.out

test: all
	@echo "\nKILL\n"
	@./catcher.out KILL &
	@./sender.out `pidof catcher.out` $n KILL
	@echo "\nSIGQUEUE\n"
	@./catcher.out SIGQUEUE &
	@./sender.out `pidof catcher.out` $n SIGQUEUE
	@echo "\nSIGRT\n"
	@./catcher.out SIGRT &
	@./sender.out `pidof catcher.out` $n SIGRT

kill_procs:
	ps -aux | grep er.out | grep -v grep | awk '{print $$2}' | xargs kill -s 9

clean:
	rm -Rf *.out

