
cmake_minimum_required(VERSION 2.8)

project(cl)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -std=c++11 -O0 -Wall -g -ggdb ")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -std=c++11 -O3 -Wall ")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")

add_executable(cl clMain.cpp)
target_link_libraries(cl OpenCL)
