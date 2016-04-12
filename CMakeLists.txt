cmake_minimum_required(VERSION 2.8)
project(ArchiHW1)

set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -g -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}") #default: Debug
#Do not use -O3!!
set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -Os -std=c++11")

set(UTILS_SOURCE_FILES
        Utils.cpp
        utils/RawBufferHandle.cpp)

set(SOURCE_FILES
        RawBinary.cpp
        Instruction.cpp
        ExecutionEngine.cpp
        Context.cpp Context.h
        ${UTILS_SOURCE_FILES}
        Task.cpp
        Types.h
        Types.cpp)
include_directories(./include)

if(NOT DEFINED OUT_EXE_NAME)
    set(OUT_EXE_NAME pipline)
endif()

add_executable(${OUT_EXE_NAME} ${SOURCE_FILES} main.cpp)

add_executable(${OUT_EXE_NAME}Tests ${SOURCE_FILES} tests/TestMain.cpp )

add_custom_target(install-test-data ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tests/test_dataset ${CMAKE_BINARY_DIR}/test_dataset
        COMMENT "Install dataset folders...")
#Clean dataset either
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/test_dataset)