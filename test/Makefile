CC=clang

.PHONY: all
all: clean build_dir factorial factorial2

.PHONY: build_dir
build_dir:
	mkdir -p ../build/test

factorial: factorial_opt.ll
	afl-clang-fast ../build/test/$^ -o ../build/test/$@

factorial2: factorial.c
	clang -fpass-plugin=../build/src/libcollectfunctiontypes.so $^ -o ../build/test/$@

factorial_opt.ll: factorial.ll
	opt -load ../build/src/librecordfunctionio.so \
		-load-pass-plugin ../build/src/librecordfunctionio.so \
		-passes="record-function-io" \
		-record-output-file="records.txt" \
		../build/test/factorial.ll -S -o ../build/test/$@ \

factorial.ll: factorial.c
	$(CC) -O1 -S -emit-llvm $^ -o ../build/test/$@

.PHONY: clean
clean:
	rm -rf ../build/test
