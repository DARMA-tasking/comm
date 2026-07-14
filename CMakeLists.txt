cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

file(READ "VERSION" _comm_version_str)
string(STRIP "${_comm_version_str}" _comm_version_str)
project(comm VERSION ${_comm_version_str})

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
endif()

include(cmake/load_doxygen.cmake)

# External libraries

# Bundled fmt: kept as an embedded build (its vendored CMakeLists has the
# install path stripped). comm folds the fmt target into its own export set
# (see src/CMakeLists.txt) so downstream consumers get it transitively.
set(FMT_LIBRARY fmt)
add_subdirectory(lib/fmt)

# Bundled magistrate ships its own checkpoint/magistrate package configs
# (magistrate_enable_export_of_targets defaults ON), which define the
# `vt::lib::checkpoint` target consumed transitively by comm.
set(kokkos_DISABLE ON CACHE BOOL "" FORCE)
set(kokkos_kernels_DISABLE ON CACHE BOOL "" FORCE)
add_subdirectory(lib/magistrate)

# Project components

set(COMM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(src)

set(COMM_EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)
add_subdirectory(examples)

set(COMM_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
enable_testing()
add_subdirectory(tests)

# Package config so downstream projects can `find_package(comm CONFIG)`
if(vt_backend_enabled)
  set(COMM_HAS_VT 1)
else()
  set(COMM_HAS_VT 0)
endif()

configure_file(
  cmake/commConfig.cmake.in
  "${PROJECT_BINARY_DIR}/commConfig.cmake" @ONLY
)

install(
  FILES        "${PROJECT_BINARY_DIR}/commConfig.cmake"
  DESTINATION  cmake
)
