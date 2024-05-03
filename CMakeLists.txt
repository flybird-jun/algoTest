#cmake .. -G "MinGW Makefiles"
cmake_minimum_required(VERSION 3.22)
project(algoTest)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out/bin)

option(ALGO "compiler src code" ON)

option(TEST "compiler test" OFF)

if (ALGO)
    add_subdirectory(src)
endif()

if(TEST)
    add_subdirectory(test)
endif()