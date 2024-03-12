CC=clang
CXX=clang++
CXXFLAGS=$(shell llvm-config --cxxflags)
LDFLAGS=$(shell llvm-config --ldflags)

.PHONY: all
all: clean factorial

factorial: factorial_opt.ll
	$(CC) $^ -o $@

factorial_opt.ll: libtest.so factorial.ll
	opt -load-pass-plugin libtest.so -passes=print-function-io -disable-output -o $@

libtest.so: print_function_io_pass.cpp
	$(CXX) -shared -fPIC $(CXXFLAGS) $(LDFLAGS) $^ -o $@

factorial.ll: factorial.c
	$(CC) -S -emit-llvm $^ -o $@

.PHONY: clean
clean:
	rm -rf *.o libtest.so factorial.ll factorial_opt.ll factorial
