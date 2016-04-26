cmake_minimum_required(VERSION 2.8)
project(ArchiHW1)

set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -g -std=c++11")
#Do not use -O3!!
set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -Os -std=c++11")

set(TASKS_SOURCE_FILES
        tasks/Tasks.cpp
        tasks/RTasks.cpp tasks/RTasks.h
        tasks/ITasks.cpp tasks/ITasks.h
        tasks/JTasks.cpp tasks/JTasks.h )

set(TEST_SOURCE_HEADERS
        tests/TestCase.h
        tests/TestUtils.h
        tests/TestInstruction.h
        tests/TestAdts.h tests/TestTasks.h)

set(SOURCE_FILES
        RawBinary.cpp RawBinary.h
        Instruction.cpp Instruction.h
        ExecutionEngine.cpp ExecutionEngine.h
        Context.cpp Context.h
        adts/RawBufferHandle.cpp adts/RawBufferHandle.h
        adts/BlockingQueue.cpp adts/BlockingQueue.h
        adts/ScopedReadWriteLock.cpp adts/ScopedReadWriteLock.h
        Utils.cpp Utils.h
        Task.cpp Task.h
        ${TASKS_SOURCE_FILES}
        Types.cpp Types.h
        tasks/TaskHandle.cpp tasks/TaskHandle.h
        IFEngine.cpp IFEngine.h)
include_directories(include)

set(LINK_BOOST_LIBS
        libboost_system.a
        libboost_chrono.a
        libboost_date_time.a)
if (APPLE)
    link_directories(${CMAKE_SOURCE_DIR}/lib/darwin)
    set(LINK_BOOST_LIBS
            ${LINK_BOOST_LIBS}
            libboost_thread-mt.a
            libboost_atomic-mt.a)
elseif(WIN32)
    message(FATAL_ERROR "Not support windows yet")
else()
    link_directories(${CMAKE_SOURCE_DIR}/lib/linux)
    set(LINK_BOOST_LIBS
            ${LINK_BOOST_LIBS}
            libboost_thread.a
            libboost_atomic.a)
endif ()

if(NOT DEFINED OUT_EXE_NAME)
    set(OUT_EXE_NAME pipeline)
endif()

# Verbose level under Debug type
# Value: Trace, Normal, Quiet
if(DEFINED VERBOSE_LEVEL)
    if(VERBOSE_LEVEL STREQUAL "Trace")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DVERBOSE_LEVEL=2")

    elseif(VERBOSE_LEVEL STREQUAL "Normal")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DVERBOSE_LEVEL=1")

    elseif(VERBOSE_LEVEL STREQUAL "Quiet")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DVERBOSE_LEVEL=0")

    else()
        message(FATAL_ERROR "VERBOSE_LEVEL values: Trace, Normal, Quiet")
    endif()
else()
    #Default: Normal
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DVERBOSE_LEVEL=1")
endif()

add_executable(${OUT_EXE_NAME}
        ${SOURCE_FILES} main.cpp)
target_link_libraries(${OUT_EXE_NAME} ${LINK_BOOST_LIBS})

add_executable(${OUT_EXE_NAME}Tests
        ${SOURCE_FILES} ${TEST_SOURCE_HEADERS} tests/TestMain.cpp )
target_link_libraries(${OUT_EXE_NAME}Tests ${LINK_BOOST_LIBS})

add_custom_target(install-test-data ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/tests/test_dataset ${CMAKE_BINARY_DIR}/test_dataset
        COMMENT "Install dataset folders...")
#Clean dataset either
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/test_dataset)