cmake_minimum_required(VERSION 3.16)
project(lsmtree)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCDIR)

add_executable(testskiplist ${PROJECT_SOURCE_DIR}/test/testskiplist.cpp ${SRCDIR})

