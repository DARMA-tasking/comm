set(COMM_EXAMPLES comm_examples)

file(GLOB EXAMPLE_SOURCES "${COMM_EXAMPLES_DIR}/*.cc")

foreach(EXAMPLE_FULL ${EXAMPLE_SOURCES})
  get_filename_component(EXAMPLE ${EXAMPLE_FULL} NAME_WE)

  add_executable(${EXAMPLE} ${COMM_EXAMPLES_DIR}/${EXAMPLE}.cc)

  target_link_libraries(${EXAMPLE} PRIVATE comm)
  target_link_libraries(${EXAMPLE} PRIVATE fmt)

  target_include_directories(${EXAMPLE} PRIVATE ${COMM_SOURCE_DIR})
endforeach()
