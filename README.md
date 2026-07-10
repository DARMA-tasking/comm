# comm => communicator

## Included workflows

* See [.github/workflows/README](.github/workflows/README.md)

## Usage

```bash
# Building
cmake -S . -B build

# Compiling
cmake --build build

# Testing
./build/tests/comm_tests

## Optional multi-rank test run
mpirun -np 2 ./build/tests/comm_tests

# Examples
./build/examples/[filename]
# example: ./build/examples/dummy1

# Documentation
xdg-open build/html/index.html # Linux
open build/html/index.html # macOS
```

## Upgrade external libraries procedure

* See [lib/UPDATE.md](lib/UPDATE.md)
