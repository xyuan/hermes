project(example-schroedinger-eigen-refine-towards-origin)
add_executable(${PROJECT_NAME} main.cpp)
include (${hermes3d_SOURCE_DIR}/CMake.common)
set_common_target_properties(${PROJECT_NAME})
set(BIN ${PROJECT_BINARY_DIR}/${PROJECT_NAME})
add_test(schroedinger-eigen-refine-towards-origin ${BIN} 1)
add_test(schroedinger-eigen-refine-uniformly  ${BIN} 2)
