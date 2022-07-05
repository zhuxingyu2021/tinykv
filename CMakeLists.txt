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

add_executable(testskiplist ${PROJECT_SOURCE_DIR}/test/testskiplist.cpp)
target_link_libraries(testskiplist lsmtree)
add_executable(testsstable ${PROJECT_SOURCE_DIR}/test/testsstable.cpp)
target_link_libraries(testsstable lsmtree)
add_executable(testcorrectness ${PROJECT_SOURCE_DIR}/test/testcorrectness.cpp)
target_link_libraries(testcorrectness lsmtree)
