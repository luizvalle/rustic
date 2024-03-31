# Coreutils branch coverage analysis

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

## 2: Modify the Makefile to include coverage information at compilation

```sh
sed -i 's/^CFLAGS = .*$/CFLAGS = -g -O0 --coverage -fprofile-abs-path -Wno-uninitialized -fkeep-inline-functions -fkeep-static-functions/' Makefile
sed -i 's/^LDFLAGS = .*$/LDFLAGS = --coverage/' Makefile
```
