cmake_minimum_required(VERSION 3.8)
project(videotransform)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_subdirectory(example)
endif()

