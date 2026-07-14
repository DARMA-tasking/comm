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
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
  $<INSTALL_INTERFACE:include>
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

#
# Install / export rules so downstream projects can `find_package(comm CONFIG)`
# and link `comm::comm`. Transitive deps (MPI, magistrate/checkpoint, fmt) are
# resolved by consumers via find_dependency in commConfig.cmake.
#

install(
  TARGETS               ${COMM_LIBRARY}
  EXPORT                ${COMM_LIBRARY}
  LIBRARY DESTINATION   lib
  ARCHIVE DESTINATION   lib
  RUNTIME DESTINATION   bin
  INCLUDES DESTINATION  include
)

# Fold the embedded fmt target into comm's export set so `fmt::fmt` resolves
# transitively (as `comm::fmt`) without a separate fmt package config.
install(
  TARGETS               ${FMT_LIBRARY}
  EXPORT                ${COMM_LIBRARY}
  LIBRARY DESTINATION   lib
  ARCHIVE DESTINATION   lib
  RUNTIME DESTINATION   bin
  INCLUDES DESTINATION  include
)

install(
  DIRECTORY    ${CMAKE_SOURCE_DIR}/lib/fmt/include/fmt
  DESTINATION  include
  FILES_MATCHING PATTERN "*.h"
)

install(
  EXPORT       ${COMM_LIBRARY}
  DESTINATION  cmake
  NAMESPACE    comm::
  FILE         "commTargets.cmake"
)

# Public headers (mirrors the <comm/...> include layout) and generated config.
install(
  DIRECTORY      ${CMAKE_CURRENT_SOURCE_DIR}/comm
  DESTINATION    include
  FILES_MATCHING PATTERN "*.h"
)

install(
  FILES        ${CMAKE_CURRENT_BINARY_DIR}/generated/comm/config/cmake_config.h
  DESTINATION  include/comm/config
)

# Export to the build tree as well (usable without installing).
export(
  TARGETS    ${COMM_LIBRARY} ${FMT_LIBRARY}
  FILE       "commTargets.cmake"
  NAMESPACE  comm::
)
