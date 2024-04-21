# Test case to function map analysis with gcov

## 1: Prepare for gcov analysis

Follow the steps in [BRANCH_COVERAGE.md](BRANCH_COVERAGE.md) to prepare
coreutils for gcov analysis.

## 2: Save gcov information for each coreutuls test case separately

Run the following bash script from the coreutils root directory:

```sh
DIR=<directory>
find tests/ -type f -name "*.sh" | while IFS= read -r test_script; do
    rm -f src/*.gcda ./*.gcov.json.gz # Remove existing coverage information
    make check TESTS=$test_script # Execute the test script
    executable=$(basename "$(dirname "$test_script")") # Get the directory name of the sh script
    gcov "src/$executable" --json-format # Get the coverage information for the executable
    subdirname="${test_script//\//_}"  # Replace slashes with underscores
    subdirname="${subdirname%.sh}"  # Remove ".sh" extension
    mkdir -p $DIR/$subdirname # Create a directory to save the gcov files
    mv ./*.gcov.json.gz $DIR/$subdirname # Move the gcov data to the directory
    gzip -d $DIR/$subdirname/*.gz
done
```
