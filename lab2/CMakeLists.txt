cmake_minimum_required(VERSION 3.22)

project(local_chat)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/lib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/host)
