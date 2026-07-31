# comm => communicator

## Included workflows

* See [.github/workflows/README](.github/workflows/README.md)

## Read the documentation

To learn *comm*, read the
[full documentation](https://darma-tasking.github.io/comm_docs/html/index.html)
that is automatically generated whenever a push occurs to "master".

## Get code

```bash
git clone git@github.com:DARMA-tasking/comm.git
```

## Required

- MPI, installed and accessible via CC/CXX
- Ninja, installed and accessible
- `magistrate`, (`vt` ecosystem), cloned like:
```bash
cd comm/lib
git clone git@github.com:DARMA-tasking/magistrate.git
```

## Usage

### With scripts

In `comm/`:
```bash
# Building
./ci/build_cpp.sh "$PWD" "$PWD/build/ci"

# Testing
./ci/test_cpp.sh "$PWD" "$PWD/build/ci"
```

### With cmake

In `comm/`:
```bash
# Building
cmake -S . -B build

# Compiling
cmake --build build
cmake --build build --parallel # Faster

# Installing
cmake --install build --prefix "$PWD/install"

# Testing
./build/tests/comm_tests
ctest --test-dir build --output-on-failure

## Optional multi-rank test run
mpirun -np 2 ./build/tests/comm_tests

# Examples
./build/examples/[filename]
# Example: ./build/examples/dummy1

# Documentation
xdg-open build/html/index.html # Linux
open build/html/index.html # macOS
```

## Upgrade external libraries procedure

* See [lib/UPDATE.md](lib/UPDATE.md)
