# Converting coreutils to Rust with C2Rust

## 1. Install C2Rust

Follow the instructions in the
[c2rust repository](https://github.com/immunant/c2rust/blob/master/README.md).

Remember to update the `PATH` shell variable so that the shell can find
the `c2rust` executables. For example:

```sh
export PATH=$PATH:/home/luiz/.cargo/bin
```

## 2. Download and prepare coreutils

Repository: [link](https://github.com/coreutils/coreutils).

### 2.1. Download

Run the following command:

```sh
git clone git@github.com:coreutils/coreutils.git
```

### 2.2 Configure the repository

Go into the `coreutils` directory created from the command above:

```sh
cd coreutils
```

Run the following commands from the `coreutils` root:

```sh
./bootstrap
./configure --quiet
```

## 3. Translate coreutils

Reference: [c2rust README](https://github.com/immunant/c2rust/blob/master/README.md).

### 3.1 Make some modifications to the Makefile

According to the `c2rust` README, they do not support some compiler, so we
remove them to be safe:

```sh
sed -i 's/-O[0-3]//' Makefile
```

Also, it seems that `c2rust` does not support some warning flags used by
`coreutis`, so we remove those as well:

```sh
sed -i 's/\(-W[^ ]*\)//g' Makefile
```

### 3.1 Generate the `compile_commands.json` file

This is a
[compilation database](https://clang.llvm.org/docs/JSONCompilationDatabase.html)
file used by c2rust when transpiling the code.

First, install `scan-build`, which will generate the compilation database file
from `coreutil`'s Makefile:

```sh
sudo pip install scan-build
```

The run this command to create the file:

```sh
make clean && intercept-build make all
```

After this command runs, you should have the `compile_commands.json` file in the
`coreutils` root directory.

### 3.2 Transpile the code

Run the following command from the `coreutils` root directory:

```sh
c2rust transpile \
--emit-build-files \
--overwrite-existing \
--output-dir=c2rust/ \
compile_commands.json
```

This will generate the Rust files in the `c2rust/` directory within the
`coreutils` root directory.
