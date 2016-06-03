cmake_minimum_required(VERSION 2.8)
project(ArchiHW1)

set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -g -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}") #default: Debug
#Do not use -O3!!
set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -Os -std=c++11")

set(TEST_HEADER_FILES 
        tests/TestCase.h
        tests/TestUtils.h
        tests/TestInstruction.h)

set(SOURCE_FILES
        RawBinary.cpp
        Instruction.cpp
        ExecutionEngine.cpp
        Context.cpp Context.h
        adts/RawBufferHandle.cpp
        Utils.cpp
        Task.cpp
        Types.h
        Types.cpp
        CMP.hpp CMP.cpp)
include_directories(./include)

if(NOT DEFINED OUT_EXE_NAME)
    set(OUT_EXE_NAME cmp)
endif()

add_executable(${OUT_EXE_NAME} ${SOURCE_FILES} main.cpp)

add_executable(${OUT_EXE_NAME}Tests 
               ${SOURCE_FILES} ${TEST_HEADER_FILES} tests/TestMain.cpp )

add_custom_command(TARGET ${OUT_EXE_NAME}Tests POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/tests/test_dataset $<TARGET_FILE_DIR:${OUT_EXE_NAME}Tests>/test_dataset
                  COMMENT "Install dataset folders...")
