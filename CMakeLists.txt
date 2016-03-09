cmake_minimum_required(VERSION 3.4)
project(ArchiHW1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        main.cpp
        RawBinary.cpp RawBinary.h
        Instruction.cpp Instruction.h
        Types.h
        tests/TestCase.h tests/TestInstruction.h
        misc/Log.cpp misc/Log.h
        ExecutionEngine.cpp ExecutionEngine.h
        Phase.cpp Phase.h)
include_directories(./include)

add_executable(ArchiHW1 ${SOURCE_FILES})