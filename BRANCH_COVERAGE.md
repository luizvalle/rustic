# Coreutils branch coverage analysis with gcov

[Gcov Documentation](https://gcc.gnu.org/onlinedocs/gcc/gcov/introduction-to-gcov.html#introduction-to-gcov)

## 1: Clone and setup coreutils

### 1.1 Clone

```sh
git clone git@github.com:coreutils/coreutils.git
```

Go into the `coreutils` repository root:

```sh
cd coreutils
```

### 1.2 Extract files needed to build

```sh
./bootstrap
```

### 1.3 Configure

```sh
./configure --quiet
```

## 2: Modify the Makefile to include coverage information

Run the following commands to edit the `Makefile` in the coreutils root:

```sh
sed -i 's/^CFLAGS = .*$/CFLAGS = -g -O0 --coverage -fprofile-abs-path -Wno-uninitialized -fkeep-inline-functions -fkeep-static-functions/' Makefile
sed -i 's/^LDFLAGS = .*$/LDFLAGS = --coverage/' Makefile
```

For an explanation of each of the flags, see `cc` man page:

```sh
man gcc
```

## 3: Running the end-to-end tests in coreutils

Coverage information will be automatically recorded when the executables run.

The coreutils repository contains a directory with shell scripts testing
different behaviors of each of the executables. These scripts can be found in
the `tests/` directory.

To run all tests, run the following command from coreutils root:

```sh
make check tests
```

## 4: Exporting coverage information to a JSON

### File explanation

Once all the tests complete, two types of files will be populated for each
original object file:

* `.gcno`: Contain information to reconstruct the basic block graphs and assign
source line numbers to blocks.
* `.gcda`: Contains the execution counts.

See the [documentation](https://gcc.gnu.org/onlinedocs/gcc/gcov/brief-description-of-gcov-data-files.html#brief-description-of-gcov-data-files) for more details

### Generating the JSON files

To extract the data as a JSON format
([format description](https://gcc.gnu.org/onlinedocs/gcc/gcov/invoking-gcov.html#cmdoption-gcov-j)),
run the following command (replacing `<directory>` with the directory to place
the JSON files in):

```sh
DIR=<directory>
find src/ -executable -type f | while IFS= read -r file; do
    gcov "$file" --json-format
    json_file_name="$(basename $file).gcov.json"
    gzip -d "$json_file_name.gz"
    mkdir -p $DIR
    mv "$json_file_name" <directory>
done
```

## Appendix

### Clean the coreutils directory

From the coreutils repository root, run the following commands

```sh
make clean
rm src/*.gcda src/*.gcno
```
