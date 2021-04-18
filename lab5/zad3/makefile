cc = gcc -Wall
pipe = pipe.fifo
data = data
res = res.txt
exp_res = expected_res.txt
N = 3

all: consumer producer

producer: producer.c
	$(cc) producer.c -o producer.out

consumer: consumer.c
	$(cc) consumer.c -o consumer.out

test_data:
	@mkdir -p data
	python3 -c 'with open("data/data1.txt", "w") as f: f.write("a"*$(N)*3)'
	python3 -c 'with open("data/data2.txt", "w") as f: f.write("b"*$(N)*3)'
	python3 -c 'with open("data/data3.txt", "w") as f: f.write("c"*$(N)*3)'
	python3 -c 'with open("data/data4.txt", "w") as f: f.write("d"*$(N)*3)'
	python3 -c 'with open("data/data5.txt", "w") as f: f.write("e"*$(N)*3)'

test_prep: test_data
	@rm -f "$(pipe)"
	@mkfifo "$(pipe)"
	@echo "\n" > $(res)

test_many_to_one: test_prep
	@echo "Test many to one with N = $(N)" >> wnioski.txt
	@for row in 1 2 3 4 5; do \
	./producer.out "$(pipe)" $$row "data/$(data)$$row.txt" $(N) &\
	done
	@./consumer.out "$(pipe)" "$(res)" $(N)
	@rm -f expected_res.txt
	@for row in 1 2 3 4 5; do \
	cat "data/$(data)$$row.txt" >> $(exp_res); echo '' >> $(exp_res); \
	done
	@make verify --no-print-directory

test_one_to_many: test_prep
	@echo "Test one to many with N = $(N)" >> wnioski.txt
	@for row in 1; do \
	./producer.out "$(pipe)" $$row "data/$(data)$$row.txt" $(N) &\
	done
	@for row in 1 2 3 4; do \
	./consumer.out "$(pipe)" "$(res)" $(N) &\
	done
	@./consumer.out "$(pipe)" "$(res)" $(N)
	@rm -f expected_res.txt
	@for row in 1; do \
	cat "data/$(data)$$row.txt" >> $(exp_res); echo '' >> $(exp_res); \
	done
	@make verify --no-print-directory

test_many_to_many: test_prep
	@echo "Test many to many with N = $(N)" >> wnioski.txt
	@for row in 1 2 3 4 5; do \
	./producer.out "$(pipe)" $$row "data/$(data)$$row.txt" $(N) &\
	done
	@for row in 1 ; do \
	./consumer.out "$(pipe)" "$(res)" $(N) &\
	done
	@./consumer.out "$(pipe)" "$(res)" $(N)
	@rm expected_res.txt
	@for row in 1 2 3 4 5; do \
	cat "data/$(data)$$row.txt" >> $(exp_res); echo '' >> $(exp_res); \
	done
	@make verify --no-print-directory

test_N:
	echo "Starting test with N=$(N)"
	@make test_many_to_one --no-print-directory
	@make test_one_to_many --no-print-directory
	@make test_many_to_many --no-print-directory

test: all
	@rm -f wnioski.txt
	@touch wnioski.txt
	@make test_N N=5
	@make test_N N=10
	@make test_N N=50
	@make test_N N=500
	@make test_N N=1500
	@make test_N N=3000
	@make test_N N=5000
	@make test_N N=16000
	@make test_N N=65600

verify: res.txt
	@sed -i '/^$$/d' $(exp_res)
	@sed -i '/^$$/d' $(res)
	@if cmp --silent $(res) $(exp_res); then \
		echo '    Test passed - files are the same' >> wnioski.txt; \
	else \
		echo '    Test failed - files differ' >> wnioski.txt; \
	fi

clean:
	rm -Rf *.out data pipe.fifo $(exp_res) $(res)

