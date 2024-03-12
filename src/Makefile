CC=clang
CXX=clang++
CXXFLAGS=$(shell llvm-config --cxxflags)
LDFLAGS=$(shell llvm-config --ldflags)

.PHONY: all
all: clean factorial

factorial: factorial_opt.ll
	$(CC) $^ -o $@

factorial_opt.ll: libprintfunctionio.so factorial.ll
	opt -load-pass-plugin ./libprintfunctionio.so -passes=print-function-io factorial.ll -S -o $@

libprintfunctionio.so: print_function_io_pass.cpp
	$(CXX) -shared -fPIC $(CXXFLAGS) $(LDFLAGS) $^ -o $@

factorial.ll: factorial.c
	$(CC) -O0 -S -emit-llvm $^ -o $@

.PHONY: clean
clean:
	rm -rf *.o libprintfunctionio.so factorial.ll factorial_opt.ll factorial
