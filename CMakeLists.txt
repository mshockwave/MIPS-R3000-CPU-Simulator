cmake_minimum_required(VERSION 3.4)
project(ArchiHW1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        RawBinary.cpp
        Instruction.cpp
        ExecutionEngine.cpp
        Task.cpp
        Utils.cpp)
include_directories(./include)

add_executable(ArchiHW1 ${SOURCE_FILES} main.cpp)

add_executable(ArchiHW1Tests ${SOURCE_FILES} tests/TestMain.cpp )