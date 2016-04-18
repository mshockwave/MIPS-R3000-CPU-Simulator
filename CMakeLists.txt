cmake_minimum_required(VERSION 2.8)
project(ArchiHW1)

set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -g -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}") #default: Debug
#Do not use -O3!!
set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -Os -std=c++11")

set(SOURCE_FILES
        RawBinary.cpp RawBinary.h
        Instruction.cpp Instruction.h
        ExecutionEngine.cpp ExecutionEngine.h
        Context.cpp Context.h
        adts/RawBufferHandle.cpp adts/RawBufferHandle.h
        adts/FlipFlop.h
        Utils.cpp Utils.h
        tasks/FirstClassTasks.cpp tasks/FirstClassTasks.h
        tasks/FragmentTasksR.cpp tasks/FragmentTasksR.h
        tasks/FragmentTasksI.cpp tasks/FragmentTasksI.h
        tasks/FragmentTasksJ.cpp tasks/FragmentTasksJ.h
        tasks/InstructionMap.cpp
        tasks/Task.h tasks/Task.cpp
        Task.h
        Types.cpp Types.h)
include_directories(./include)

if(NOT DEFINED OUT_EXE_NAME)
    set(OUT_EXE_NAME pipeline)
endif()

add_executable(${OUT_EXE_NAME} ${SOURCE_FILES} main.cpp)

add_executable(${OUT_EXE_NAME}Tests ${SOURCE_FILES} tests/TestMain.cpp )

add_custom_target(install-test-data ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tests/test_dataset ${CMAKE_BINARY_DIR}/test_dataset
        COMMENT "Install dataset folders...")
#Clean dataset either
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/test_dataset)