CXX=clang++
CXXFLAGS=$(shell llvm-config --cxxflags)
LDFLAGS=$(shell llvm-config --ldflags)

.PHONY: all
all: clean build_dir librecordfunctionio.so libcollectionfunctiontypestats.so

.PHONY: build_dir
build_dir:
	mkdir ../build/src -p

librecordfunctionio.so: record_function_io.cpp
	$(CXX) -shared -fPIC $(CXXFLAGS) $(LDFLAGS) $^ -o ../build/src/$@

libcollectionfunctiontypestats.so: collect_function_type_stats.cpp
	$(CXX) -shared -fPIC $(CXXFLAGS) $(LDFLAGS) $^ -o ../build/src/$@

.PHONY: clean
clean:
	rm -rf ../build/src
