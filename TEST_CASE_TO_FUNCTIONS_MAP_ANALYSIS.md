# Test case to function map analysis with gcov

## 1: Prepare for gcov analysis

Follow the steps in [BRANCH_COVERAGE.md](BRANCH_COVERAGE.md) to prepare
coreutils for gcov analysis.

## 2: Save gcov information for each coreutuls test case separately

Run the following bash script from the coreutils root directory:

```sh
DIR=<directory>
find tests/ -type f -name "*.sh" | while IFS= read -r test_script; do
    rm src/*.gcda src/*.json # Remove existing coverage information
    make check TESTS=$test_script # Execute the test script
    find src/ -executable -type f | while IFS= read -r executable; do
        gcov "$executable" --json-format
    done
    subdirname="${test_script//\//_}"  # Replace slashes with underscores
    subdirname="${subdirname%.sh}"  # Remove ".sh" extension
    mkdir -p $DIR/$subdirname # Create a directory to save the gcov files
    mv src/*.gcda $DIR/$subdirname
    mv src/*.gcov.json $DIR/$subdirname
done
```
