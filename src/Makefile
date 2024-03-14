CXX=clang++
CXXFLAGS=$(shell llvm-config --cxxflags)
LDFLAGS=$(shell llvm-config --ldflags)

.PHONY: all
all: clean build_dir librecordfunctionio.so


.PHONY: build_dir
build_dir:
	mkdir ../build/src -p

librecordfunctionio.so: record_function_io.cpp
	$(CXX) -shared -fPIC $(CXXFLAGS) $(LDFLAGS) $^ -o ../build/src/$@

.PHONY: clean
clean:
	rm -rf ../build/src
