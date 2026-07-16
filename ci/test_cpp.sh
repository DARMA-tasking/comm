#!/usr/bin/env bash

set -euo pipefail
set -x

if (( $# != 2 )); then
    echo "Usage: $0 <source-dir> <build-root>" >&2
    exit 2
fi

export COMM=$1
export COMM_BUILD=$2/comm

if [[ ! -f "${COMM_BUILD}/CTestTestfile.cmake" ]]; then
    echo "Comm test build not found: ${COMM_BUILD}" >&2
    exit 2
fi

ctest \
    --test-dir "${COMM_BUILD}" \
    --output-on-failure \
    2>&1 | tee "${COMM_BUILD}/cmake-output.log"
