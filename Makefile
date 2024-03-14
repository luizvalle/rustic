.PHONY: all
all: clean src test

.PHONY: src
src:
	make -C src

.PHONY: test
test:
	make -C test

.PHONY: clean
clean:
	make clean -C src && make clean -C test
