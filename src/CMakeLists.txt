set(COMM_LIBRARY comm)

find_package(MPI REQUIRED)

if(vt_backend_enabled)
  set(vt_backend_feature_enabled 1)
  find_package(vt REQUIRED CONFIG)
else()
  set(vt_backend_feature_enabled 0)
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/comm/config/cmake_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/generated/comm/config/cmake_config.h @ONLY
)

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/comm/*.h)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/comm/*.cc)

add_library(
  ${COMM_LIBRARY}
  STATIC
  ${HEADER_FILES} ${SOURCE_FILES}
)

target_include_directories(
  ${COMM_LIBRARY} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(
  ${COMM_LIBRARY} PUBLIC
    MPI::MPI_CXX
    vt::lib::checkpoint
    fmt::fmt
)

if(vt_backend_enabled)
  target_link_libraries(${COMM_LIBRARY} PUBLIC vt::vt)
endif()
