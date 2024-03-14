CC=clang
CXX=clang++
CXXFLAGS=$(shell llvm-config --cxxflags)
LDFLAGS=$(shell llvm-config --ldflags)

.PHONY: all
all: clean factorial

factorial: factorial_opt.ll
	$(CC) $^ -o $@

factorial_opt.ll: librecordfunctionio.so factorial.ll
	opt -load-pass-plugin ./librecordfunctionio.so -passes=record-function-io factorial.ll -S -o $@

librecordfunctionio.so: record_function_io.cpp
	$(CXX) -shared -fPIC $(CXXFLAGS) $(LDFLAGS) $^ -o $@

factorial.ll: factorial.c
	$(CC) -O0 -S -emit-llvm $^ -o $@

.PHONY: clean
clean:
	rm -rf *.o librecordfunctionio.so factorial.ll factorial_opt.ll factorial
