cc = gcc -Wall
pipe = pipe.fifo
data = data
res = res.txt
buff = 3

all: consumer producer

producer: producer.c
	$(cc) producer.c -o producer.out

consumer: consumer.c
	$(cc) consumer.c -o consumer.out

test_data:
	@mkdir -p data
	@echo -n 'aaaaaaaaaaaaaaa' > data/data1.txt
	@echo -n 'bbbbbbbbbbbbbbb' > data/data2.txt
	@echo -n 'ccccccccccccccc' > data/data3.txt
	@echo -n 'ddddddddddddddd' > data/data4.txt
	@echo -n 'eeeeeeeeeeeeeee' > data/data5.txt

test: all test_data
	@rm -f "$(pipe)"
	@mkfifo "$(pipe)"
	@echo "\n" > $(res)
	@for row in 1 2 3 4 5; do \
	./producer.out "$(pipe)" $$row "data/$(data)$$row.txt" $(buff) &\
	done
	@./consumer.out "$(pipe)" "$(res)" $(buff)
# 	@diff -w test.txt <(echo $$(sed -E 's/^#[0-9]+#//g' conf.txt)) > /dev/null && echo 'test passed' || echo 'test failed'

clean:
	rm -Rf *.out data

