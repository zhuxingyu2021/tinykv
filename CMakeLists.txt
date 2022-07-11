cmake_minimum_required(VERSION 3.16)
project(lsmtree)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_VERBOSE_MAKEFILE ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O1")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCDIR)

add_library(lsmtree SHARED ${SRCDIR})
target_link_libraries(lsmtree PRIVATE Threads::Threads)

add_executable(TestCorrectness ${PROJECT_SOURCE_DIR}/test/TestCorrectness.cpp)
target_link_libraries(TestCorrectness lsmtree)
add_executable(BenchmarkSequantial ${PROJECT_SOURCE_DIR}/test/BenchmarkSequantial.cpp)
target_link_libraries(BenchmarkSequantial lsmtree)
add_executable(BenchmarkRandom ${PROJECT_SOURCE_DIR}/test/BenchmarkRandom.cpp)
target_link_libraries(BenchmarkRandom lsmtree)
