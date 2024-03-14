.PHONY: all
all: clean src test

.PHONY: src
src:
	make -C src

.PHONY: test
test:
	make -C test

.PHONY: fuzz
fuzz: src test
	afl-fuzz -i test_cases/ -o afl_output/ ./build/test/factorial

.PHONY: clean
clean:
	make clean -C src && make clean -C test && rm -f records.txt
