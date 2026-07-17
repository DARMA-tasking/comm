find_package(GTest QUIET)

if (NOT GTest_FOUND)
  include(FetchContent)
  # Do not let the fetched googletest add its own install rules to `comm`'s
  # install (otherwise `cmake --install` tries to install libgmock/libgtest).
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )
  FetchContent_MakeAvailable(googletest)
endif()

# Setup tests

set(COMM_TESTS comm_tests)

file(GLOB_RECURSE TEST_SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

add_executable(${COMM_TESTS} ${TEST_SOURCE_FILES})

target_link_libraries(${COMM_TESTS}
  PRIVATE
    comm
    GTest::gtest
    fmt::fmt
)

target_include_directories(${COMM_TESTS}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_test(NAME ${COMM_TESTS} COMMAND ${COMM_TESTS})
