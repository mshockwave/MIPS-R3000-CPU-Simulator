cmake_minimum_required(VERSION 3.4)
project(ArchiHW1)

set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -g -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}") #default: Debug
set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE} -Os -std=c++11")

set(SOURCE_FILES
        RawBinary.cpp
        Instruction.cpp
        ExecutionEngine.cpp
        Context.cpp Context.h
        Task.cpp
        Types.h
        Types.cpp
        Utils.cpp)
include_directories(./include)

add_executable(ArchiHW1 ${SOURCE_FILES} main.cpp)

add_executable(ArchiHW1Tests ${SOURCE_FILES} tests/TestMain.cpp )

add_custom_target(install-test-data ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tests/test_dataset ${CMAKE_BINARY_DIR}/test_dataset
        COMMENT "Install dataset folders...")
#Clean dataset either
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/test_dataset)