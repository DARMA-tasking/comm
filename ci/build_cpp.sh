#!/usr/bin/env bash

set -euo pipefail
set -x

if (( $# != 2 )); then
    echo "Usage: $0 <source-dir> <build-root>" >&2
    exit 2
fi

source_dir=$1
build_root=$2

magistrate_rev=${MAGISTRATE_REV:-develop}
magistrate_source=${source_dir}/lib/magistrate
comm_build=${build_root}/comm
generator=${CMAKE_GENERATOR:-Ninja}

if [[ ! -f "${source_dir}/CMakeLists.txt" ]]; then
    echo "Comm source directory not found: ${source_dir}" >&2
    exit 2
fi

# Comm embeds Magistrate with add_subdirectory(lib/magistrate). A regular
# checkout does not contain it because the directory is intentionally ignored.
if [[ ! -f "${magistrate_source}/CMakeLists.txt" ]]; then
    if [[ -e "${magistrate_source}" ]]; then
        echo "${magistrate_source} exists but is not a Magistrate source tree" >&2
        exit 2
    fi

    mkdir -p "${source_dir}/lib"
    git clone \
        --branch "${magistrate_rev}" \
        --depth 1 \
        https://github.com/DARMA-tasking/magistrate.git \
        "${magistrate_source}"
fi

if command -v ccache >/dev/null 2>&1; then
    echo "=== ccache statistics before build ===" >&2
    ccache --show-stats
else
    echo "=== ccache not found; compiling without it ===" >&2
fi

mkdir -p "${comm_build}"

cmake_command=(
    cmake
    -S "${source_dir}"
    -B "${comm_build}"
    -G "${generator}"
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}"
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    -DBUILD_SHARED_LIBS="${BUILD_SHARED_LIBS:-OFF}"
    -Dvt_backend_enabled="${VT_BACKEND_ENABLED:-OFF}"
    -DCOMM_DOXYGEN_ENABLED="${COMM_DOXYGEN_ENABLED:-0}"
)

if command -v ccache >/dev/null 2>&1; then
    cmake_command+=(-DCMAKE_CXX_COMPILER_LAUNCHER=ccache)
fi

"${cmake_command[@]}" 2>&1 | tee "${comm_build}/cmake-configure.log"

if test "${COMM_DOXYGEN_ENABLED:-0}" -eq 1
then
    MCSS=${comm_build}/m.css
    GHPAGE=${comm_build}/DARMA-tasking.github.io

    git clone --depth=1 "https://x-access-token:${GITHUB_TOKEN}@github.com/DARMA-tasking/DARMA-tasking.github.io" "${GHPAGE}"
    git clone https://github.com/mosra/m.css "${MCSS}"
    git -C "${MCSS}" checkout 699abdd5
    "$MCSS/documentation/doxygen.py" "${comm_build}/Doxyfile-mcss"

    if test "${GIT_BRANCH:-}" = "7-build-doc"
    then
        CKPT_NAME=comm_docs

        git -C "${GHPAGE}" rm -r --ignore-unmatch "${CKPT_NAME}"
        mv "${comm_build}/docs" "${GHPAGE}/${CKPT_NAME}"

        cd "$GHPAGE"
        git config --global user.email "jliffla@sandia.gov"
        git config --global user.name "Jonathan Lifflander"
        git add "$CKPT_NAME"
        git commit --allow-empty -m "Update comm_docs (auto-build)"
        git push origin master
    fi
else
    echo "=== compiling comm ===" >&2
    build_command=(cmake --build "${comm_build}")
    "${build_command[@]}" 2>&1 | tee "${comm_build}/compilation-output.log"
fi

if command -v ccache >/dev/null 2>&1; then
    echo "=== ccache statistics after build ===" >&2
    ccache --show-stats
fi
