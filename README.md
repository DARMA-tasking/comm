# comm => communicator

## Included workflows

* See [.github/workflows/README](.github/workflows/README.md)

## Get code

```bash
git clone git@github.com:DARMA-tasking/comm.git
```

## Required

- MPI, installed and accessible via CC/CXX
- `magistrate`, (`vt` ecosystem), cloned like:
```bash
cd comm/lib
git clone git@github.com:DARMA-tasking/magistrate.git
```

## Usage

In `comm/`:
```bash
# Building
cmake -S . -B build

# Compiling
cmake --build build
cmake --build build --parallel # Faster

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
