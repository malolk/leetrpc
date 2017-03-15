cmake_minimum_required(VERSION 2.6)

project(leetrpc C CXX)

set(CXX_FLAGS
	-g
    -march=native
    -std=c++0x
    -pthread
    -rdynamic
	)

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/deps/glue)
include_directories(${CMAKE_SOURCE_DIR}/deps)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(deps)
add_subdirectory(src)

add_subdirectory(demo)
