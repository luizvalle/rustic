# RustiC
The goal of this project is to collect a parallel dataset consisting of programs
implemented equivalently in C and Rust. To this end, we propose a pipeline,
RustiC, that takes the C source code of a program and converts it to idiomatic
and as safe as possible Rust. We focus on real-world system-level programs such
as those present in [coreutils](https://github.com/coreutils/coreutils).

## Pipeline overview
The final pipeline will include the high-level steps described below. This
assumes that we have collected unit tests and end-to-end tests for the
program.

1. Translate the entire C program to Rust using
[c2rust](https://github.com/immunant/c2rust). This results in Rust code that is
neither idiomatic or safe, but it compiles and behaves the same as the original
C code. Additionally, there is a one-to-one mapping between the C and Rust
functions.
2. For each function in the c2rust translation, do the following:
   a. Extract the function, the imports used by the function, the function
   call sites, and the global variables used by the function.
   b. Ask GPT-4 (or any other instruction-tuned LLM) to refactor the function
   and call sites so that the resulting code is (whenever possible) safe, uses
   Rust data types and libraries, and behaves the same as the original code.
   c. Replace the refactored function, call sites, and imports into the c2rust
   translation of the program.
   d. Attempt to compile the program. If this fails, provide the error message
   to the LLM and request the code of the function and call site to be updated
   to resolve the error. Repeat until compilation succeeds or a maximum number
   of retries is reached.
   e. Run unit tests on the function. If they don't pass, provide the failing
   test cases to the LLM and ask for revision of the code.
   f. Run the end-to-end tests on the compiled program. If any doesn't pass,
   provide the feedback to the LLM and ask for a revision of the code.

## Unit test collection
We instrument each function so that the inputs and output of the function are
logged when the program is executed. We then run the end-to-end tests included
in the repository and/or fuzz the program to collect comprehensive test cases.

## Current progress
We performed extensive analysis on
[coreutils](https://github.com/coreutils/coreutils) to understand the types
of function available, the coverage of the test cases, and to collect some
unit tests. We also developed a manual version of the pipeline described above
which does steps 1-2.d and verified on a handful of functions.

## Future work
* Collect more unit tests using fuzzing and developing a way to log use-defined
types and the contents of memory pointed to by pointers.
* Automate the translation pipeline.
* Find a way to provide feedback to the LLM on end-to-end test failure.
* Understand what types of functions the proposed method struggles with.
* Find a way to translate functions that are interdependent.
* Measure the naturallness of the resulting translation when compared to
human-written code.
* Expand the scope to other system-level C programs.

## Install the dependencies

The Python packages needed to run the project are listed in the
[requirements.txt](requirements.txt) file. To install all the packages, run the
following command from the root of the repository:

```sh
pip install -r requirements.txt
```

## Notebooks

The notebooks used to analyze the data can be found in the
[notebooks/](notebooks) directory.

* [FunctionTypeAnalaysis.ipynb](notebooks/FunctionTypeAnalaysis.ipynb): Contains
the analysis for the types of functions available in
[coreutils](https://github.com/coreutils/coreutils).
* [CoverageAnalysis.ipynb](notebooks/CoverageAnalysis.ipynb): Contains the
basic-block and function coverage analysis for the end-to-end tests included in
[coreutils](https://github.com/coreutils/coreutils).

The [notebooks/data/](notebooks/data/) directory contains some data used by the
scripts.

## LLVM passes

The LLVM passes used to instrument and collect data on the
[coreutils](https://github.com/coreutils/coreutils) functions can be found in
[llvm\_passes](llvm_passes/).

* [collect\_function\_types.cpp](llvm_passes/collect_function_types.cpp): LLVM
Module Pass that iterates through every function in each module during compile
time and prints out to stderr the function signature as follows:
   ```
   <function_name>,args:[<arg1_type>;<arg2_type>;...],ret:<ret_type>
   ```
* [record\_function\_io.cpp](llvm_passes/record_function_io.cpp): LLVM Function
Pass that instruments each function with an `fprintf()` call before each return
statement. The `fprintf()` call logs the inputs and output of the function to a
file provided as a pass argument. Each line of the file has the
following format:
   ```
   name:<function_name>,input:<input1_type>:<input1_value>,input:<input2_type>:<input2_value>:...,output:<output_type>:<output_value>
   ```

To compile the passes, run the [`llvm_passes/Makefile`](llvm_passes/Makefile).
It produces two shared objects.

To include the pass in the compilation (which needs to use
[clang](https://clang.llvm.org/)), first compile the program to LLVM IR:

```sh
$ clang -O1 -S -emit-llvm foo.c -o foo.ll
```

Where `foo.c` is the name of the C file.

For the function type pass, compile the LLVM IR using the following command:

```sh
$ opt -load <path to librecordfunctionio.so> \
-load-pass-plugin <path to librecordfunctionio.so> \
-passes="record-function-io" \
-record-output-file=<output file path> \
foo.ll -S -o foo_opt.ll

$ clang foo_opt.ll -o foo
```

For the collection function types pass, use the following command:

```sh
clang -fpass-plugin=<path to libcollectfunctiontypes.so> foo.c -o foo
```

## RustiC pipeline
Found in [pipeline](pipeline/).

So far, we only have one script:

* [refactor.py](pipeline/refactor.py): It accepts the function definition,
call sites, global variables, and imports. It then returns the refactored
version produced by a GPT-X model (see the script for all parameters).

## Other instructions

The [instructions/](instructions/) directory contains the following:

* [C2RUST\_COREUTILS.md](instructions/C2RUST_COREUTILS.md): Instructions to
translate coreutils to Rust using [c2rust](https://github.com/immunant/c2rust).
Because of the compleity of the coreutils Makefile, c2rust is only capable of
converting the source files but not producing a working Cargo package. To do so
one must extract each source C file for a program and create a simple Makefile
like the one below. Then follow step 3.1 in this instruction to generate a
working Cargo package for that program.

   ```
   CFLAGS=-I../coreutils/lib -I../coreutils/src
   SRCS := $(wildcard *.c)
   OBJS := $(SRCS:.c=.o)
   
   .PHONY: all
   all: clean echo
   
   echo: $(OBJS)
   	gcc $^ -o $@
   
   %.o: %.c
   	gcc $(CFLAGS) -c $< -o $@
   
   .PHONY: clean
   clean:
   	rm -rf $(OBJS) echo
   ```

* [BRANCH\_COVERAGE.md](instructions/BRANCH_COVERAGE.md): Instructions to run
[gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html) on coreutils and get the
function and branch coverage data.
* [TEST\_CASE\_TO\_FUNCTIONS\_MAP\_ANALYSIS.md](instructions/TEST_CASE_TO_FUNCTIONS_MAP_ANALYSIS.md):
Instructions to get mapping from each of the coreutils functions to the test
case that exercises it.
