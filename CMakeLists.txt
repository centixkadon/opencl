
cmake_minimum_required(VERSION 2.8)

project(cl C CXX ASM)

find_package(OpenCV REQUIRED)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -std=c++11 -O0 -Wall -g -ggdb ")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -std=c++11 -O3 -Wall ")

add_executable(cl clMain.cpp)
target_link_libraries(cl OpenCL ${OpenCV_LIBS} pthread)
